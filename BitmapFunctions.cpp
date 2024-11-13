#include "EditorBMP.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>

using uint8_t = unsigned char;
using uint32_t = unsigned int;
using uint16_t = unsigned short int ;
using int32_t = signed int;
using int16_t = signed short int;

void Bitmap::open(const std::string &filename)
{
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);

    if (file.fail())
    {
        std::cerr << filename << "input.bmp does not open" << std::endl;
        return;
    }

    //bmpfile magic;  
    file.read((char*)(magic), sizeof(magic));

    if (magic[0] != 'B' || magic[1] != 'M')
    {
        std::cerr << filename << "wrong bmp format" << std::endl;
        return;
    }

    //bmpfile header;
    file.read(reinterpret_cast<char*>(&file_size), sizeof(file_size));
    file.read(reinterpret_cast<char*>(&creator1), sizeof(creator1));
    file.read(reinterpret_cast<char*>(&creator2), sizeof(creator2));
    file.read(reinterpret_cast<char*>(&bmp_offset), sizeof(bmp_offset));

    //bmpfile dib_info;
    file.read(reinterpret_cast<char*>(&header_size), sizeof(header_size));
    file.read(reinterpret_cast<char*>(&width), sizeof(width));
    file.read(reinterpret_cast<char*>(&height), sizeof(height));
    file.read(reinterpret_cast<char*>(&num_planes), sizeof(num_planes));
    file.read(reinterpret_cast<char*>(&bits_per_pixel), sizeof(bits_per_pixel));
    file.read(reinterpret_cast<char*>(&compression), sizeof(compression));
    file.read(reinterpret_cast<char*>(&bmp_byte_size), sizeof(bmp_byte_size));
    file.read(reinterpret_cast<char*>(&hres), sizeof(hres));
    file.read(reinterpret_cast<char*>(&vres), sizeof(vres));
    file.read(reinterpret_cast<char*>(&num_colors), sizeof(num_colors));
    file.read(reinterpret_cast<char*>(&num_important_colors), sizeof(num_important_colors));

    if (bits_per_pixel != 24)
    {
        std::cerr << filename << " uses " << bits_per_pixel
                  << "bits per pixel (only 24 bit is supported so far)" << std::endl;
        return;
    }

    if (compression != 0)
    {
        std::cerr << filename << " is compressed."
                  << "EditorBMP only supports uncompressed images" << std::endl;
        return;
    }

    int totalMemoryNeeded = width * height * 3;

    std::cout << "File memory size: " << totalMemoryNeeded << " bite" << std::endl;

    file.seekg(bmp_offset, std::ios::beg);

    pixel_data.resize(height * width * 3);

    for (int row = height - 1; row >= 0; --row)
    {
        for (int col = 0; col < width; ++col)
        {
            uint8_t blue = file.get();
            uint8_t green = file.get();
            uint8_t red = file.get();

            int index = (row * width + col) * 3;
            pixel_data[index] = red;
            pixel_data[index + 1] = green;
            pixel_data[index + 2] = blue;
        }

        int padding = (4 - (width * 3) % 4) % 4;
        file.ignore(padding);
    }

    file.close();
}



void Bitmap::save(const std::string &filename)
{
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);

    if (file.fail())
    {
        std::cerr << filename << "Can not be opened for editing." << std::endl;
        return;
    }

    if (!(width > 0 && height > 0))
    {
        std::cerr << "Bitmap cannot be saved. It is not a valid image." << std::endl;
        return;
    }

    bmp_offset = sizeof(magic) + sizeof(file_size) + sizeof(creator1) + sizeof(creator2) +
                 sizeof(header_size) + sizeof(width) + sizeof(height) +
                 sizeof(num_planes) + sizeof(bits_per_pixel);

    file.write((char*)(&magic), sizeof(magic));

    file.write(reinterpret_cast<char*>(&file_size), sizeof(file_size));
    file.write(reinterpret_cast<char*>(&creator1), sizeof(creator1));
    file.write(reinterpret_cast<char*>(&creator2), sizeof(creator2));
    file.write(reinterpret_cast<char*>(&bmp_offset), sizeof(bmp_offset));

    file.write(reinterpret_cast<char*>(&header_size), sizeof(header_size));
    file.write(reinterpret_cast<char*>(&width), sizeof(width));
    file.write(reinterpret_cast<char*>(&height), sizeof(height));
    file.write(reinterpret_cast<char*>(&num_planes), sizeof(num_planes));
    file.write(reinterpret_cast<char*>(&bits_per_pixel), sizeof(bits_per_pixel));
    file.write(reinterpret_cast<char*>(&compression), sizeof(compression));
    file.write(reinterpret_cast<char*>(&bmp_byte_size), sizeof(bmp_byte_size));
    file.write(reinterpret_cast<char*>(&hres), sizeof(hres));
    file.write(reinterpret_cast<char*>(&vres), sizeof(vres));
    file.write(reinterpret_cast<char*>(&num_colors), sizeof(num_colors));
    file.write(reinterpret_cast<char*>(&num_important_colors), sizeof(num_important_colors));

    for (int row = height - 1; row >= 0; --row)
    {
        for (int col = 0; col < width; col++)
        {
            int index = (row * width + col) * 3;
            uint8_t blue = pixel_data[index];
            uint8_t green = pixel_data[index + 1];
            uint8_t red = pixel_data[index + 2];

            file.put(red);
            file.put(green);
            file.put(blue);
        }
        size_t padding = (4 - (width * 3) % 4) % 4;
        for (size_t i = 0; i < padding; i++)
        {
            file.put(0);
        }
    }
    file.close();
}

void Bitmap::Rotate(bool clockwise)
{
    int new_width = height;
    int new_height = width;

    std::vector<uint8_t> new_pixel_data(new_width * new_height * 3);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int old_index = (y * width + x) * 3;

            int new_x, new_y;
            if (clockwise)
            {
                new_x = new_width - 1 - y;
                new_y = x;
            }
            else
            {
                new_x = y;
                new_y = new_height - 1 - x;                
            }

            int new_index = (new_y * new_width + new_x) * 3;

            new_pixel_data[new_index] = pixel_data[old_index];
            new_pixel_data[new_index + 1] = pixel_data[old_index + 1];
            new_pixel_data[new_index + 2] = pixel_data[old_index + 2];
        }
    }

    width = new_width;
    height = new_height;
    pixel_data.swap(new_pixel_data);
}