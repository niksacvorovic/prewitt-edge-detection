use std::fs::File;
use std::env;
use std::io::{Read, Seek, Write};
use std::path::Path;

fn main() -> Result<(), Box<dyn std::error::Error>>{
    let args: Vec<String> = env::args().collect();
    let inputpath = Path::new(&args[1]);
    let outputpath = Path::new(&args[2]);
    let mut input = File::open(inputpath)?;
    let mut output = File::create(outputpath)?;
    input.seek(std::io::SeekFrom::Start(10))?;
    let mut buffer: [u8; 4] = [0; 4];
    input.read_exact(&mut buffer)?;
    let header_size: u32 = u32::from_ne_bytes(buffer);
    input.seek(std::io::SeekFrom::Start(18))?;
    input.read_exact(&mut buffer)?;
    let mut width: u32 = u32::from_ne_bytes(buffer);
    input.read_exact(&mut buffer)?;
    let mut height: u32 = u32::from_ne_bytes(buffer);
    input.seek(std::io::SeekFrom::Start(0))?;
    let mut header = vec![0u8; header_size.try_into().unwrap()];
    input.read_exact(&mut header)?;
    output.write(&header)?;
    output.seek(std::io::SeekFrom::Start(18))?;
    width -= 2;
    height -= 2;
    let width_buf = width.to_ne_bytes();
    let height_buf = height.to_ne_bytes();
    output.write(&width_buf)?;
    output.write(&height_buf)?;
    output.seek(std::io::SeekFrom::Start(header_size as u64))?;
    let mut pixel_buffer: [Vec<u8>; 3] = [vec![0u8; width as usize], vec![0u8; width as usize], vec![0u8; width as usize]];
    let mut rgb: [u8; 3] = [0, 0, 0];
    for i in 0..2{
        for j in 0..width{
            input.read_exact(&mut rgb)?;
            pixel_buffer[i][j as usize] = (rgb[2] as f32 * 0.299 + rgb[1] as f32 * 0.587 + rgb[0] as f32 * 0.299) as u8;
        }
    }
    width += 2;
    height += 2;
    let mut row_length: u32 = (width - 2) * 3;
    while row_length % 4 != 0 {
        row_length += 1;
    }
    let mut convx: i16;
    let mut convy: i16;
    let mut conv: u8;
    let mut output_buffer = vec![0u8; row_length as usize];
    for _i in 1..(height - 1){
        pixel_buffer[2] = vec![0u8; width as usize];
        for j in 0..width{
            input.read_exact(&mut rgb)?;
            pixel_buffer[2][j as usize] = (rgb[2] as f32 * 0.299 + rgb[1] as f32 * 0.587 + rgb[0] as f32 * 0.114) as u8;
        }
        for j in 1..(width - 3){
            convx = pixel_buffer[0][(j + 1) as usize] as i16 + pixel_buffer[1][(j + 1) as usize] as i16 + pixel_buffer[2][(j + 1) as usize] as i16 - 
                    pixel_buffer[0][(j - 1) as usize] as i16 - pixel_buffer[1][(j - 1) as usize] as i16 - pixel_buffer[2][(j - 1) as usize] as i16;
            convy = pixel_buffer[2][(j - 1) as usize] as i16 + pixel_buffer[2][j as usize] as i16 + pixel_buffer[2][(j + 1) as usize] as i16 - 
                    pixel_buffer[0][(j - 1) as usize] as i16 - pixel_buffer[0][j as usize] as i16 - pixel_buffer[0][(j + 1) as usize] as i16;
            conv = f32::sqrt(convx as f32 * convx as f32 + convy as f32 * convy as f32) as u8;
            for k in 0..3{
                output_buffer[((j - 1) * 3 + k) as usize] = conv;
            }
        }
        output.write(&output_buffer)?;
        pixel_buffer[0] = std::mem::take(&mut pixel_buffer[1]);
        pixel_buffer[1] = std::mem::take(&mut pixel_buffer[2]);
    }
    Ok(())
}
