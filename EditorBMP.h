#ifndef EDITORBMP_H
#define EDITORBMP_H

#include <string>
#include <vector>

class Pixel
{
public:

    int red, green, blue;


    Pixel() : red(0), green(0), blue(0) { }


    Pixel(int r, int g, int b) : red(r), green(g), blue(b) { }
};

typedef std::vector<std::vector<Pixel>> PixelMatrix;

class Bitmap
{
private:
    PixelMatrix pixels;

public:

    void Rotate(bool clockwise, const PixelMatrix &original);

    void applyGaussianFilter(PixelMatrix &image);

    void open(const std::string &namebmp);

    void save(const std::string &namebmp);

    bool isImage();

    PixelMatrix toPixelMatrix();

    void fromPixelMatrix(const PixelMatrix &);
};

#endif
