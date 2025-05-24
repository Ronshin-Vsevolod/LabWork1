#include "EditorBMP.h"
#include <chrono>
#include <iostream>
#include <omp.h>
#include <vector>
#include <algorithm>
#include <fstream>

void createLargeTestImage(const std::string& filename, int width, int height)
{
    std::cout << "Creating test image " << width << "x" << height << "..." << std::endl;
    
    uint8_t magic[2] = {'B', 'M'};
    struct BMPFileHeader
    {
        uint32_t file_size;
        uint16_t creator1;
        uint16_t creator2;
        uint32_t bmp_offset;
    };
    BMPFileHeader fileHeader;
    
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
    BMPFileDIBInfo dibInfo;
    
    int row_size = width * 3;
    int padding = (4 - (row_size % 4)) % 4;
    int stride = row_size + padding;
    
    fileHeader.bmp_offset = sizeof(magic) + sizeof(BMPFileHeader) + sizeof(BMPFileDIBInfo);
    fileHeader.file_size = fileHeader.bmp_offset + (stride * height);
    fileHeader.creator1 = 0;
    fileHeader.creator2 = 0;
    
    dibInfo.header_size = 40;
    dibInfo.width = width;
    dibInfo.height = height;
    dibInfo.num_planes = 1;
    dibInfo.bits_per_pixel = 24;
    dibInfo.compression = 0;
    dibInfo.bmp_byte_size = stride * height;
    dibInfo.hres = 2835;
    dibInfo.vres = 2835;
    dibInfo.num_colors = 0;
    dibInfo.num_important_colors = 0;
    
    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to create test image" << std::endl;
        return;
    }
    
    file.write((char*)magic, sizeof(magic));
    file.write(reinterpret_cast<char*>(&fileHeader), sizeof(BMPFileHeader));
    file.write(reinterpret_cast<char*>(&dibInfo), sizeof(BMPFileDIBInfo));
    
    std::vector<uint8_t> row_data(stride, 0);
    
    for (int y = height - 1; y >= 0; --y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint8_t r = static_cast<uint8_t>((x * 255) / width);
            uint8_t g = static_cast<uint8_t>((y * 255) / height);
            uint8_t b = static_cast<uint8_t>(((x+y) * 255) / (width+height));
            
            row_data[x*3] = b;
            row_data[x*3+1] = g;
            row_data[x*3+2] = r;
        }
        
        file.write(reinterpret_cast<char*>(row_data.data()), stride);
    }
    
    file.close();
    std::cout << "Test image created successfully" << std::endl;
}

double runTest(int num_threads, const std::string& input_file)
{
    omp_set_num_threads(num_threads);
    std::cout << "\n=== Running with " << num_threads << " threads ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    Bitmap bitmap;
    bitmap.open(input_file);

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
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    double time = duration.count();
    
    std::cout << "Execution time: " << time << " seconds" << std::endl;
    return time;
}

int main(int argc, char* argv[])
{
    std::cout << "===== Image Processing Performance Testing =====" << std::endl;
    
    std::string input_file = "test_image.bmp";
    
    if (argc > 1 && std::string(argv[1]) == "large")
    {
        createLargeTestImage(input_file, 2048, 2048);
    }
    else
    {
        createLargeTestImage(input_file, 800, 600);
    }
    
    int max_threads = omp_get_max_threads();
    std::cout << "Maximum available threads: " << max_threads << std::endl;
    
    std::vector<std::pair<int, double>> results;
    
    results.push_back({1, runTest(1, input_file)});
    if (max_threads >= 2) results.push_back({2, runTest(2, input_file)});
    if (max_threads >= 4) results.push_back({4, runTest(4, input_file)});
    if (max_threads > 4) results.push_back({max_threads, runTest(max_threads, input_file)});
    
    std::cout << "\n===== Test Results =====" << std::endl;
    std::cout << "Threads\tTime (sec)\tSpeedup" << std::endl;
    
    double base_time = results[0].second;
    
    for (const auto& result : results)
    {
        int threads = result.first;
        double time = result.second;
        double speedup = base_time / time;
        
        std::cout << threads << "\t" << time << "\t" << speedup << "x" << std::endl;
    }
    
    return 0;
} 