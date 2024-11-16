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

    pixel_data.resize(height * width * 3);

    file.seekg(bmp_offset, std::ios::beg);

    for (int row = height - 1; row >= 0; --row)
    {
        for (int col = 0; col < width; ++col)
        {
            uint8_t blue = file.get();
            uint8_t green = file.get();
            uint8_t red = file.get();

            int index = (row * width + col) * 3;
            pixel_data[index] = blue;
            pixel_data[index + 1] = green;
            pixel_data[index + 2] = red;
        }

        int padding = (4 - (width * 3) % 4) % 4;

        if (padding > 0)
        {
            file.ignore(padding);
        }
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

    /*bmp_offset = sizeof(magic) + sizeof(file_size) + sizeof(creator1) + sizeof(creator2) +
                 sizeof(header_size) + sizeof(width) + sizeof(height) +
                 sizeof(num_planes) + sizeof(bits_per_pixel);*/

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

    file.seekp(bmp_offset, std::ios::beg);

    for (int row = height - 1; row >= 0; --row)
    {
        for (int col = 0; col < width; col++)
        {
            int index = (row * width + col) * 3;
            uint8_t blue = pixel_data[index];
            uint8_t green = pixel_data[index + 1];
            uint8_t red = pixel_data[index + 2];

            file.put(blue);
            file.put(green);
            file.put(red);
        }

        size_t padding = ((width * 3) % 4) % 4;

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



void Bitmap::applyGaussianFilter()
{
    const float kernel[5][5] =
    {
        {1/273.0f, 4/273.0f, 6/273.0f, 4/273.0f, 1/273.0f},
        {4/273.0f, 16/273.0f, 24/273.0f, 16/273.0f, 4/273.0f},
        {6/273.0f, 24/273.0f, 36/273.0f, 24/273.0f, 6/273.0f},
        {4/273.0f, 16/273.0f, 24/273.0f, 16/273.0f, 4/273.0f},
        {1/273.0f, 4/273.0f, 6/273.0f, 4/273.0f, 1/273.0f}
    };

    std::vector<uint8_t> new_pixel_data(pixel_data.size());

    int width = this->width;
    int height = this->height;

    for (int y = 2; y < height - 2; ++y)
    {
        for (int x = 2; x < width - 2; ++x)
        {
            float redSum = 0, greenSum = 0, blueSum = 0;

            for (int ki = -2; ki <= 2; ++ki)
            {
                for (int kj = -2; kj <= 2; ++kj)
                {
                    int pixelIndex = ((y + ki) * width + (x + kj)) * 3;
                    redSum += pixel_data[pixelIndex] * kernel[ki + 2][kj + 2];
                    greenSum += pixel_data[pixelIndex + 1] * kernel[ki + 2][kj + 2];
                    blueSum += pixel_data[pixelIndex + 2] * kernel[ki + 2][kj + 2];
                }
            }

            int newIndex = (y * width + x) * 3;
            new_pixel_data[newIndex] = std::min(255, std::max(0, static_cast<int>(redSum)));
            new_pixel_data[newIndex + 1] = std::min(255, std::max(0, static_cast<int>(greenSum)));
            new_pixel_data[newIndex + 2] = std::min(255, std::max(0, static_cast<int>(blueSum)));
        }
    }

    pixel_data.swap(new_pixel_data);
}