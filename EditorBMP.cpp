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

int main()
{
    Bitmap bitmap;

    bitmap.open("input.bmp");

    {
        Bitmap tempBitmap = bitmap;
        tempBitmap.Rotate(0);
        tempBitmap.save("RotatedCounterClockwise.bmp");
    }

    {
        bitmap.Rotate(1);
        bitmap.save("RotatedClockwise.bmp");

        bitmap.applyGaussianFilter();
        bitmap.save("Filtered.bmp");
    }

    return 0;
}