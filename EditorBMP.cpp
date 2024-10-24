#include <iostream>
#include <fstream>

struct BMPHeader
{
    unsigned char header[54];
    int width;
    int height;
    short bitsPerPixel;
};

size_t CalcMemorySize(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error: file does not open" << std::endl;
        return 0;
    }

    BMPHeader bmpHeader;
    file.read(reinterpret_cast<char*>(&bmpHeader.header), 54);

    bmpHeader.width = *reinterpret_cast<int*>(&bmpHeader.header[18]);
    bmpHeader.height = *reinterpret_cast<int*>(&bmpHeader.header[22]);
    bmpHeader.bitsPerPixel = *reinterpret_cast<short*>(&bmpHeader.header[28]);

    size_t pixelArraySize = bmpHeader.width * bmpHeader.height * (bmpHeader.bitsPerPixel / 8);
    size_t totalSize = pixelArraySize + sizeof(bmpHeader.header);

    return totalSize;
}

int main()
{
    const char* filename = "namebmp.bmp";
    size_t memorySize = CalcMemorySize(filename);

    if (memorySize > 0)
    {
        std::cout << "File memory size: " << memorySize << " byte" << std::endl;
    }

    return 0;
}
