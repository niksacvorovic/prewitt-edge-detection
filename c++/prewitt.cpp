#include <iostream>
#include <fstream>
#include <memory>
#include <cmath>

int main(int argc, char* argv[]){
    std::ifstream input(argv[1], std::ios::binary);
    std::ofstream output(argv[2], std::ios::binary);
    unsigned int size, width, height;
    input.seekg(10);
    input.read((char*)&size, sizeof(int));
    input.seekg(18);
    input.read((char*)&width, sizeof(int));
    input.read((char*)&height, sizeof(int));
    input.seekg(0);
    char* header = new char[size];
    input.read(header, size);
    output.write(header, size);
    output.seekp(18);
    width -= 2;
    height -= 2;
    output.write((char*)&width, sizeof(int));
    output.write((char*)&height, sizeof(int));  
    output.seekp(size);
    delete[] header;
    std::unique_ptr<unsigned char> buffer[3] = {
        std::unique_ptr<unsigned char>(new unsigned char[width]),
        std::unique_ptr<unsigned char>(new unsigned char[width])
    };
    width += 2;
    height += 2;
    unsigned char r, g, b;
    for(int i = 0; i < 2; ++i){
        for(int j = 0; j < width; ++j){
            input.read((char*)&b, 1);
            input.read((char*)&g, 1);
            input.read((char*)&r, 1);
            buffer[i].get()[j] = r * 0.299 + g * 0.587 + b * 0.114;
        }
        for(int j = 0; (width * 3 + j) % 4 != 0; ++j) input.seekg(input.tellg() += 1);
    }
    short convx, convy, conv;
    for(int i = 1; i < height - 1; ++i){
        buffer[2] = std::unique_ptr<unsigned char>(new unsigned char[width]);
        for(int j = 0; j < width; ++j){
            input.read((char*)&b, 1);
            input.read((char*)&g, 1);
            input.read((char*)&r, 1);
            buffer[2].get()[j] = r * 0.299 + g * 0.587 + b * 0.114;
        }
        for(int j = 0; (width * 3 + j) % 4 != 0; ++j) input.seekg(input.tellg() += 1);
        for(int j = 1; j < width - 1; ++j){
            convx = buffer[0].get()[j + 1] + buffer[1].get()[j + 1] + buffer[2].get()[j + 1] + 
                    - buffer[0].get()[j - 1] - buffer[1].get()[j - 1] - buffer[2].get()[j - 1];
            convy = buffer[2].get()[j - 1] + buffer[2].get()[j] + buffer[2].get()[j + 1] + 
                    - buffer[0].get()[j - 1] - buffer[0].get()[j] - buffer[0].get()[j + 1];
            conv = sqrt(convx * convx + convy * convy);
            if(conv != (unsigned char) conv) conv = 255;
            for(int k = 0; k < 3; ++k){
                output.write((char*) &conv, 1);
            }
        }
        for(int j = 0; ((width - 2) * 3 + j) % 4 != 0; ++j) output << '\x00';
        buffer[0] = std::move(buffer[1]);
        buffer[1] = std::move(buffer[2]);
    }
}