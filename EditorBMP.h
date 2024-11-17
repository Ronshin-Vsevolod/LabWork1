
#ifndef EDITORBMP_H
#define EDITORBMP_H

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>

class Bitmap
{
private:

    uint8_t magic[2];

    struct BMPFileHeader
    {
        uint32_t file_size;
        uint16_t creator1;
        uint16_t creator2;
        uint32_t bmp_offset;
    };

    struct BMPFileDIBInfo
    {
        uint32_t header_size;
        int32_t width;
        int32_t height;
        uint16_t num_planes;
        uint16_t bits_per_pixel;
        uint32_t compression;
        uint32_t bmp_byte_size;
        int32_t hres;
        int32_t vres;
        uint32_t num_colors;
        uint32_t num_important_colors;
    };

    std::vector<uint8_t> pixel_data;

    BMPFileHeader fileHeader;
    BMPFileDIBInfo dibInfo;

public:

    void open(const std::string &namebmp);

    void save(const std::string &namebmp);

    void Rotate(bool clockwise);

    void applyGaussianFilter(int kernelSize);

};

#endif