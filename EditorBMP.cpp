#include "EditorBMP.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>

typedef unsigned char uchar_t;
typedef unsigned int uint32_t;
typedef unsigned short int uint16_t;
typedef signed int int32_t;
typedef signed short int int16_t;

const int MIN_RGB = 0;
const int MAX_RGB = 255;
const int BMP_MAGIC_ID = 2;

struct bmpfile_magic
{
    uchar_t magic[BMP_MAGIC_ID];
};

struct bmpfile_header
{
    uint32_t file_size;
    uint16_t creator1;
    uint16_t creator2;
    uint32_t bmp_offset;
};

struct bmpfile_dib_info
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

void Bitmap::open(const std::string &filename)
{
    std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);

    pixels.clear();

    if (file.fail())
    {
        std::cerr << filename << "input.bmp does not open" << std::endl;
        return;
    }

    bmpfile_magic magic;
    file.read((char*)(&magic), sizeof(magic));

    if (magic.magic[0] != 'B' || magic.magic[1] != 'M')
    {
        std::cerr << filename << "wrong bmp format" << std::endl;
        return;
    }

    bmpfile_header header;
    file.read((char*)(&header), sizeof(header));

    bmpfile_dib_info dib_info;
    file.read((char*)(&dib_info), sizeof(dib_info));

    bool flip = true;
    if (dib_info.height < 0)
    {
        flip = false;
        dib_info.height = -dib_info.height;
    }

    if (dib_info.bits_per_pixel != 24)
    {
        std::cerr << filename << " uses " << dib_info.bits_per_pixel
                  << "bits per pixel (only 24 bit is supported so far)" << std::endl;
        return;
    }

    if (dib_info.compression != 0)
    {
        std::cerr << filename << " is compressed."
                  << "EditorBMP only supports uncompressed images" << std::endl;
        return;
    }

    int totalMemoryNeeded = dib_info.width * dib_info.height * 3;

    std::cout << "File memory size: " << totalMemoryNeeded << " bite" << std::endl;

    file.seekg(header.bmp_offset);

    for (int row = 0; row < dib_info.height; row++)
    {
        std::vector<Pixel> row_data;

        for (int col = 0; col < dib_info.width; col++)
        {
            int blue = file.get();
            int green = file.get();
            int red = file.get();

            row_data.push_back(Pixel(red, green, blue));
        }

        file.seekg(dib_info.width % 4, std::ios::cur);

        if (flip)
        {
            pixels.insert(pixels.begin(), row_data);
        }
        else
        {
            pixels.push_back(row_data);
        }
    }
    file.close();
}

void Bitmap::save(const std::string &filename)
{
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);

    if (file.fail())
    {
        std::cerr << filename << "could not be opened for editing." << std::endl;
        return;
    }

    if (!isImage())
    {
        std::cerr << "Bitmap cannot be saved. It is not a valid image." << std::endl;
        return;
    }

    bmpfile_magic magic;
    magic.magic[0] = 'B';
    magic.magic[1] = 'M';

    bmpfile_header header = {0};

    header.bmp_offset = sizeof(bmpfile_magic) + sizeof(bmpfile_header) + sizeof(bmpfile_dib_info);

    size_t rowSize = ((pixels[0].size() * 3 + 3) / 4) * 4;
    header.file_size = header.bmp_offset + pixels.size() * rowSize;

    file.write((char*)(&magic), sizeof(magic));

    file.write((char*)(&header), sizeof(header));

    bmpfile_dib_info dib_info = {0};

    dib_info.header_size = sizeof(bmpfile_dib_info);
    dib_info.width = pixels[0].size();
    dib_info.height = pixels.size();

	dib_info.num_planes = 1;
	dib_info.bits_per_pixel = 24;
	dib_info.compression = 0;
	dib_info.bmp_byte_size = 0;
	dib_info.hres = 2835;
	dib_info.vres = 2835;
	dib_info.num_colors = 0;
	dib_info.num_important_colors = 0;

	file.write((char*)(&dib_info), sizeof(dib_info));

	for (int row = pixels.size() - 1; row >= 0; row--)
	{
        const std::vector<Pixel> &row_data = pixels[row];

        for (const Pixel &pix : row_data)
        {
            file.put((uchar_t)(pix.blue));
            file.put((uchar_t)(pix.green));
            file.put((uchar_t)(pix.red));
        }

        for (size_t i = row_data.size() % 4; i < 4; i++)
        {
            file.put(0);
        }
	}

	file.close();
}

bool Bitmap::isImage()
{
	const size_t height = pixels.size();

	if (height == 0 || pixels[0].size() == 0)
	{
		return false;
	}

	const size_t width = pixels[0].size();

	for (size_t row=0; row < height; row++)
	{
		if (pixels[row].size() != width)
		{
			return false;
		}
		for (size_t column=0; column < width; column++)
		{
			Pixel current = pixels[row][column];
			if (current.red > MAX_RGB || current.red < MIN_RGB ||
			   current.green > MAX_RGB || current.green < MIN_RGB ||
			   current.blue > MAX_RGB || current.blue < MIN_RGB)
				return false;
		}
	}
	return true;
}

PixelMatrix Bitmap::toPixelMatrix()
{
	if (isImage())
	{
		return pixels;
	}
	else
	{
		return PixelMatrix();
	}
}

void Bitmap::fromPixelMatrix(const PixelMatrix &values)
{
	pixels = values;
}

void Bitmap::Rotate(bool clockwise, const PixelMatrix &original)
{
   int originalHeight = original.size();
   int originalWidth = original[0].size();

   PixelMatrix rotated(originalWidth, std::vector<Pixel>(originalHeight));

   for (int i = 0; i < originalHeight; ++i)
   {
       for (int j = 0; j < originalWidth; ++j)
       {
          if(clockwise)
          {
             rotated[j][originalHeight - 1 - i] = original[i][j];
          }
          else
          {
             rotated[originalWidth - 1 - j][i] = original[i][j];
          }
       }
   }
   pixels = rotated;
}

void applyGaussianFilter(PixelMatrix &image)
{
   const float kernel[5][5] =
   {
       {1/273.0f, 4/273.0f, 6/273.0f, 4/273.0f, 1/273.0f},
       {4/273.0f, 16/273.0f, 24/273.0f, 16/273.0f, 4/273.0f},
       {6/273.0f, 24/273.0f, 36/273.0f, 24/273.0f, 6/273.0f},
       {4/273.0f, 16/273.0f, 24/273.0f, 16/273.0f, 4/273.0f},
       {1/273.0f, 4/273.0f, 6/273.0f, 4/273.0f, 1/273.0f}
   };

   PixelMatrix result(image);

   for (size_t i = 2; i < image.size() - 2; ++i)
   {
       for (size_t j = 2; j < image[i].size() - 2; ++j)
       {
           float redSum=0, greenSum=0, blueSum=0;

           for (int ki=-2; ki <=2; ++ki)
           {
               for (int kj=-2; kj <=2; ++kj)
               {
                   const Pixel& p=image[i+ki][j+kj];
                   redSum += p.red * kernel[ki+2][kj+2];
                   greenSum += p.green * kernel[ki+2][kj+2];
                   blueSum += p.blue * kernel[ki+2][kj+2];
               }
           }

           result[i][j].red=std::min(MAX_RGB,std::max(MIN_RGB,(int)(redSum)));
           result[i][j].green=std::min(MAX_RGB,std::max(MIN_RGB,(int)(greenSum)));
           result[i][j].blue=std::min(MAX_RGB,std::max(MIN_RGB,(int)(blueSum)));
       }
   }

   image=result;
}

int main()
{
    Bitmap bitmap;

    bitmap.open("input.bmp");

    {
        Bitmap rotatedBitmap;
        rotatedBitmap.Rotate(1, bitmap.toPixelMatrix());
        rotatedBitmap.save("RotatedClockwise.bmp");
    }

    {
        Bitmap rotatedBitmap;
        rotatedBitmap.Rotate(0, bitmap.toPixelMatrix());
        rotatedBitmap.save("RotatedCounterClockwise.bmp");
    }


/*
   PixelMatrix imgMatrix = bitmap.toPixelMatrix();
   applyGaussianFilter(imgMatrix);
   bitmap.fromPixelMatrix(imgMatrix);
   bitmap.save("Filtered.bmp");
*/
   return 0;
}
