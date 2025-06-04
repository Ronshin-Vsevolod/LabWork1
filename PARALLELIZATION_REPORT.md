# Parallelization Report

## Overview

This report describes the parallelization work performed on the BMP image editor using OpenMP. The goal was to improve performance through multi-threading while maintaining correctness and ensuring no performance degradation for small images.

## Parallelized Operations

### 1. Image Rotation (`Rotate` method)
- **Target**: Pixel transformation during image rotation (clockwise/counterclockwise)
- **Implementation**: Applied `#pragma omp parallel for` to the outer loop
- **Condition**: Parallelization only activates for images larger than 1,000,000 pixels (1000×1000)
- **Rationale**: Avoids thread creation overhead for small images

### 2. Gaussian Filter (`applyGaussianFilter` method)
- **Target**: Convolution operation applying the Gaussian kernel to each pixel
- **Implementation**: Used `#pragma omp parallel for schedule(dynamic)` for better load balancing
- **Condition**: Parallelization activates for images larger than 250,000 pixels (500×500) OR kernel size ≥ 5
- **Rationale**: Dynamic scheduling handles varying computational complexity per pixel

### 3. Image Save Operation (`save` method)
- **Decision**: Reverted to original `std::copy` implementation
- **Rationale**: Parallel copying of small row data proved inefficient due to overhead

## Technical Implementation Details

### Conditional Parallelization
```cpp
if (dibInfo.width * dibInfo.height > threshold)
{
    #pragma omp parallel for
    // Parallel version
}
else
{
    // Sequential version (original code)
}
```

This approach ensures:
- Small images use the original sequential code (no performance loss)
- Large images benefit from parallelization
- Thread creation overhead is avoided when not beneficial

### Thread Management
- Main program accepts thread count as command-line argument
- Default uses maximum available threads
- Test suite automatically tests with 1, 2, 4, and maximum threads

## Testing Framework

### Performance Testing (`GoogleTest.cpp`)
- **Automatic test image generation**: Creates test images of specified sizes
- **Multiple thread configurations**: Tests with 1, 2, 4, and max threads
- **Speedup calculation**: Compares performance against single-threaded baseline
- **Two test modes**:
  - Default: 800×600 image
  - Large: 2048×2048 image for pronounced speedup demonstration

### CI Integration
- GitHub Actions workflow for automated testing
- Installs OpenMP dependencies (`libomp-dev`)
- Builds and runs performance tests automatically
- Tests both main and parallel implementation branches

## Key Optimizations

1. **Size-based parallelization**: Prevents overhead on small images
2. **Dynamic scheduling**: Better load balancing for irregular workloads
3. **Preserved original code paths**: Maintains performance for small images
4. **Self-contained testing**: No dependency on external test images

## Results

### Performance Measurements

The following results were obtained on an 8-core system using an 800×600 test image:

| Threads | Execution Time (sec) | Speedup |
|---------|---------------------|---------|
| 1       | 0.267566           | 1.00x   |
| 2       | 0.139924           | 1.91x   |
| 4       | 0.0843066          | 3.17x   |
| 8       | 0.0897168          | 2.98x   |

### Analysis

- **Near-linear speedup** up to 4 threads (3.17x speedup with 4 threads)
- **Optimal performance** achieved at 4 threads for this workload
- **Slight performance decrease** at 8 threads due to thread management overhead
- **91% parallel efficiency** at 2 threads (91.1% of theoretical maximum)
- **79% parallel efficiency** at 4 threads (79.3% of theoretical maximum)

The parallelization provides:
- **No performance degradation** for small images (uses original sequential code)
- **Significant speedup** for large images and complex operations
- **Scalable performance** with increasing thread count up to optimal point
- **Maintained correctness** - identical output regardless of thread count

## Files Modified

- `EditorBMP.cpp`: Core parallelization implementation
- `Main.cpp`: Thread count configuration via command line
- `GoogleTest.cpp`: Performance testing framework
- `Makefile`: OpenMP compilation flags
- `.github/workflows/ci.yml`: CI/CD pipeline
- `README.md`: Updated documentation

## Conclusion

The parallelization successfully improves performance for computationally intensive operations while maintaining backward compatibility and ensuring no performance regression for smaller workloads. The implementation uses intelligent thresholds to determine when parallelization is beneficial, resulting in optimal performance across all image sizes. The achieved speedup of 3.17x with 4 threads demonstrates effective utilization of multi-core systems for image processing tasks. 
