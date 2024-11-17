#include "EditorBMP.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cmath>

void Bitmap::open(const std::string &filename)
{
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);

    if (file.fail())
    {
        std::cerr << filename << "input.bmp does not open" << std::endl;
        return;
    }

    file.read((char*)(magic), sizeof(magic));

    if (magic[0] != 'B' || magic[1] != 'M')
    {
        std::cerr << filename << "wrong bmp format" << std::endl;
        return;
    }
    
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(BMPFileHeader));
    file.read(reinterpret_cast<char*>(&dibInfo), sizeof(BMPFileDIBInfo));

    if (dibInfo.bits_per_pixel != 24)
    {
        std::cerr << filename << " uses " << dibInfo.bits_per_pixel
                  << "bits per pixel (only 24 bit is supported so far)" << std::endl;
        return;
    }

    if (dibInfo.compression != 0)
    {
        std::cerr << filename << " is compressed."
                  << "EditorBMP only supports uncompressed images" << std::endl;
        return;
    }

    int totalMemoryNeeded = dibInfo.width * dibInfo.height * 3;

    std::cout << "File memory size: " << totalMemoryNeeded << " bite" << std::endl;

    pixel_data.resize(dibInfo.height * dibInfo.width * 3);

    file.seekg(fileHeader.bmp_offset, std::ios::beg);

    for (int row = dibInfo.height - 1; row >= 0; --row)
    {
        std::vector<uint8_t> bytesRow(dibInfo.width*3);
        file.read(reinterpret_cast<char*>(bytesRow.data()), dibInfo.width*3);
            
        std::copy(bytesRow.begin(), bytesRow.end(), pixel_data.begin() + (row * dibInfo.width * 3));

        int padding = (4 - (dibInfo.width * 3) % 4) % 4;

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

    if (!(dibInfo.width > 0 && dibInfo.height > 0))
    {
        std::cerr << "Bitmap cannot be saved. It is not a valid image." << std::endl;
        return;
    }

    file.write((char*)(&magic), sizeof(magic));
    file.write(reinterpret_cast<char*>(&fileHeader), sizeof(BMPFileHeader));
    file.write(reinterpret_cast<char*>(&dibInfo), sizeof(BMPFileDIBInfo));

    file.seekp(fileHeader.bmp_offset, std::ios::beg);

    for (int row = dibInfo.height - 1; row >= 0; --row)
    {
        std::vector<uint8_t> bytesRow(dibInfo.width*3);

        std::copy(pixel_data.begin() + (row * dibInfo.width * 3), pixel_data.begin() + ((row+1) * dibInfo.width * 3), bytesRow.begin());
        
        file.write(reinterpret_cast<char*>(bytesRow.data()), dibInfo.width*3);

        size_t padding = ((dibInfo.width * 3) % 4) % 4;

        for (size_t i = 0; i < padding; i++)
        {
            file.put(0);
        }
    }

    file.close();
}



void Bitmap::Rotate(bool clockwise)
{
    int new_width = dibInfo.height;
    int new_height = dibInfo.width;

    std::vector<uint8_t> new_pixel_data(new_width * new_height * 3);

    for (int y = 0; y < dibInfo.height; ++y)
    {
        for (int x = 0; x < dibInfo.width; ++x)
        {
            int old_index = (y * dibInfo.width + x) * 3;

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

    dibInfo.width = new_width;
    dibInfo.height = new_height;
    pixel_data.swap(new_pixel_data);
}



void Bitmap::applyGaussianFilter(int kernelSize)
{
    if (kernelSize % 2 == 0 || kernelSize < 3 || kernelSize > 11)
        {
            std::cerr << "Kernel size must be an odd number between 3 and 11." << std::endl;
            return;
        }

        std::vector<std::vector<float>> kernel(kernelSize, std::vector<float>(kernelSize));
        float sigma = 1.0f;
        float sum = 0.0f;

    for (int x = 0; x < kernelSize; ++x)
        {
            for (int y = 0; y < kernelSize; ++y)
            {
                int offsetX = x - kernelSize / 2;
                int offsetY = y - kernelSize / 2;
                float value = (1 / (2 * M_PI * sigma * sigma)) * std::exp(-(offsetX * offsetX + offsetY * offsetY) / (2 * sigma * sigma));
                kernel[x][y] = value;
                sum += value;
            }
        }

    for (int i = 0; i < kernelSize; ++i)
        {
            for (int j = 0; j < kernelSize; ++j)
            {
                kernel[i][j] /= sum;
            }
        }

        std::vector<uint8_t> new_pixel_data(pixel_data.size());

        for (int y = kernelSize / 2; y < dibInfo.height - kernelSize / 2; ++y)
        {
            for (int x = kernelSize / 2; x < dibInfo.width - kernelSize / 2; ++x)
            {
                float redSum = 0, greenSum = 0, blueSum = 0;

                for (int ki = 0; ki < kernelSize; ++ki)
                {
                    for (int kj = 0; kj < kernelSize; ++kj)
                    {
                        int pixelIndex = ((y + ki - kernelSize / 2) * dibInfo.width + (x + kj - kernelSize / 2)) * 3;
                        redSum += pixel_data[pixelIndex] * kernel[ki][kj];
                        greenSum += pixel_data[pixelIndex + 1] * kernel[ki][kj];
                        blueSum += pixel_data[pixelIndex + 2] * kernel[ki][kj];
                    }
                }

                int newIndex = (y * dibInfo.width + x) * 3;
                new_pixel_data[newIndex] = std::min(255, std::max(0, static_cast<int>(redSum)));
                new_pixel_data[newIndex + 1] = std::min(255, std::max(0, static_cast<int>(greenSum)));
                new_pixel_data[newIndex + 2] = std::min(255, std::max(0, static_cast<int>(blueSum)));
            }
        }

    pixel_data.swap(new_pixel_data);
}