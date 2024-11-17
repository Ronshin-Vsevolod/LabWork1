#include "EditorBMP.h"

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