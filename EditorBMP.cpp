#include "EditorBMP.h"
/*
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>
*/

const int MIN_RGB = 0;
const int MAX_RGB = 255;
const int BMP_MAGIC_ID = 2;

int main()
{
    Bitmap bitmap;

    bitmap.open("input.bmp");
/*
    {
        Bitmap rotatedBitmap;
        rotatedBitmap.Rotate(1, bitmap.toPixelMatrix());
        rotatedBitmap.save("RotatedClockwise.bmp");

        PixelMatrix imgMatrix = rotatedBitmap.toPixelMatrix();
        applyGaussianFilter(imgMatrix);
        rotatedBitmap.fromPixelMatrix(imgMatrix);
        rotatedBitmap.save("Filtered.bmp");
    }

    {
        Bitmap rotatedBitmap;
        rotatedBitmap.Rotate(0, bitmap.toPixelMatrix());
        rotatedBitmap.save("RotatedCounterClockwise.bmp");
    }
*/
    return 0;
}