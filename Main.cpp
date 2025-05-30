#include "EditorBMP.h"
#include <iostream>
#include <omp.h>
#include <chrono>

int main(int argc, char* argv[])
{
    int num_threads = omp_get_max_threads();
    
    if (argc > 1)
    {
        num_threads = std::atoi(argv[1]);
        if (num_threads <= 0 || num_threads > omp_get_max_threads())
        {
            std::cerr << "Incorrect number of threads. The default value will be used." << std::endl;
            num_threads = omp_get_max_threads();
        }
    }
    
    omp_set_num_threads(num_threads);
    std::cout << "Run using " << num_threads << "OpenMP threads" << std::endl;
    
    auto total_start = std::chrono::high_resolution_clock::now();
    
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

        bitmap.applyGaussianFilter(5);
        bitmap.save("Filtered.bmp");
    }
    
    auto total_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> total_duration = total_end - total_start;
    std::cout << "Total execution time: " << total_duration.count() << " seconds" << std::endl;

    return 0;
}