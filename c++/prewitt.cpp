#include <iostream>
#include <fstream>
#include <memory>
#include <cmath>

int main(int argc, char* argv[]){
    std::ifstream img(argv[1], std::ios::binary);
    std::ofstream prewitt(argv[2], std::ios::binary);
    int size, width, height;
    img.seekg(10);
    img.read((char*)&size, sizeof(int));
    img.seekg(18);
    img.read((char*)&width, sizeof(int));
    img.read((char*)&height, sizeof(int));
    img.seekg(0);
    char* header = new char[size];
    img.read(header, size);
    prewitt.write(header, size);
    prewitt.seekp(18);
    width -= 2;
    height -= 2;
    prewitt.write((char*)&width, sizeof(int));
    prewitt.write((char*)&height, sizeof(int));  
    prewitt.seekp(size);
    delete[] header;
    std::unique_ptr<unsigned char> buffer[3] = {
        std::unique_ptr<unsigned char>(new unsigned char[width]),
        std::unique_ptr<unsigned char>(new unsigned char[width])
    };
    width += 2;
    height += 2;
    unsigned char r, g, b, gray;
    for(int i = 0; i < 2; ++i){
        for(int j = 0; j < width; ++j){
            img.read((char*)&b, 1);
            img.read((char*)&g, 1);
            img.read((char*)&r, 1);
            buffer[i].get()[j] = (unsigned char) r * 0.299 + g * 0.587 + b * 0.114;
        }
        for(int j = 0; (width * 3 + j) % 4 != 0; ++j) img.seekg(img.tellg() + 1);
    }
    unsigned short convx, convy, conv, zero = 0;
    for(int i = 1; i < height - 1; ++i){
        buffer[2] = std::unique_ptr<unsigned char>(new unsigned char[width]);
        for(int j = 0; j < width; ++j){
            img.read((char*)&b, 1);
            img.read((char*)&g, 1);
            img.read((char*)&r, 1);
            buffer[2].get()[j] = (unsigned char) r * 0.299 + g * 0.587 + b * 0.114;
        }
        for(int j = 0; (width * 3 + j) % 4 != 0; ++j) img.seekg(img.tellg() + 1);
        for(int j = 1; j < width - 1; ++j){
            convx = buffer[0].get()[j + 1] + buffer[1].get()[j + 1] + buffer[2].get()[j + 1] + 
                    - buffer[0].get()[j - 1] - buffer[1].get()[j - 1] - buffer[2].get()[j - 1];
            convy = buffer[2].get()[j - 1] + buffer[2].get()[j] + buffer[2].get()[j + 1] + 
                    - buffer[0].get()[j - 1] - buffer[0].get()[j] - buffer[0].get()[j + 1];
            conv = (unsigned char) sqrt(convx * convx + convy * convy);
            for(int k = 0; k < 3; ++k){
                prewitt.write((char*) &conv, 1);
            }
        }
        for(int j = 0; ((width - 2) * 3 + j) % 4 != 0; ++j) prewitt.write((char*) &zero, 1);
        buffer[0] = std::move(buffer[1]);
        buffer[1] = std::move(buffer[2]);
    }
}