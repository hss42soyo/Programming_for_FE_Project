$1.$ Benchmarking

We use `std::chrono` and framework below to  measure the execution time of each baseline function.
``` cpp
start = std::chrono::high_resolution_clock::now();
// Tested Function
end = std::chrono::high_resolution_clock::now();
std::chrono::duration<double, std::milli> duration_col_major = end - start;
```
Then we test the baseline functions with various matrix and vector sizes 5 times each and calculate the average execution time and standard deviation.
``` cpp
enum class MatrixSize {
    TINY = 2,
    TINT2 = 4,
    SMALL1 = 64, 
    SMALL2 = 128,
    MEDIUM = 256, 
    MEDIUM2 = 512,
    LARGE = 1024,
    LARGE2 = 2048,
    ENORMOUS = 4096,
};
```
Which means that we test the multiplication of 2x2, 4x4,..., 4096x4096 matrices and corresponding vectors and matrices.

Below are the table of our benchmarking results.

| Function                | Size      | Average Execution Time | Standard Deviation |
|:-----------------------:|:---------:|:---------------------:|:------------------:|
| <b>multiply_mv_row_major</b>   | 2×2       | ms                    | ms                 |
|    | 4×4       | ms                    | ms                 |
|    | 64x64     | ms                    | ms                 |
|    | 128x128   | ms                    | ms                 |
|    | 256x256   | ms                    | ms                 |
|    | 512x512   | ms                    | ms                 |
|    | 1024x1024 | ms                    | ms                 |
|    | 2048x2048 | ms                    | ms                 |
|    | 4096x4096 | ms                    | ms                 |
| <b>multiply_mv_col_major</b>   | 2×2       | ms                    | ms                 |
|    | 4×4       | ms                    | ms                 |
|    | 64x64     | ms                    | ms                 |
|    | 128x128   | ms                    | ms                 |
|    | 256x256   | ms                    | ms                 |
|    | 512x512   | ms                    | ms                 |
|    | 1024x1024 | ms                    | ms                 |
|    | 2048x2048 | ms                    | ms                 |
|    | 4096x4096 | ms                    | ms                 |
| <b>multiply_mm_naive</b>   | 2×2       | ms                    | ms                 |
|    | 4×4       | ms                    | ms                 |
|    | 64x64     | ms                    | ms                 |
|    | 128x128   | ms                    | ms                 |
|    | 256x256   | ms                    | ms                 |
|    | 512x512   | ms                    | ms                 |
|    | 1024x1024 | ms                    | ms                 |
|    | 2048x2048 | ms                    | ms                 |
|    | 4096x4096 | ms                    | ms                 |
| <b>multiply_mm_transposed_b</b>   | 2×2       | ms                    | ms                 |
|    | 4×4       | ms                    | ms                 |
|    | 64x64     | ms                    | ms                 |
|    | 128x128   | ms                    | ms                 |
|    | 256x256   | ms                    | ms                 |
|    | 512x512   | ms                    | ms                 |
|    | 1024x1024 | ms                    | ms                 |
|    | 2048x2048 | ms                    | ms                 |
|    | 4096x4096 | ms                    | ms                 |


$2.$ Cache Locality Analysis

TODO

$3.$ Memory Alignment

TODO

$4.$ Inlining

In our code, we don't use small, frequently called helper functions, thus we skip the experiment with the use of the inline keyword. 

Then, we use two ways to experiment on compiling the code with and without aggressive compiler optimizations.
```
g++ -O0 -g Test_Program.cpp Original_Linear_Operation.cpp -o Test_Program.exe
```
```
g++ -O3 -g Test_Program.cpp Original_Linear_Operation.cpp -o Test_Program.exe
```
Or change the `args` in `tasks.json` in vscode.
```json
"args": [
    "-fdiagnostics-color=always",
    "-fopenmp",
    "-O3",
    "-g",
    "${fileDirname}\\*.cpp",
    "-o",
    "${fileDirname}\\${fileBasenameNoExtension}.exe"
],
```
``` json
"args": [
    "-fdiagnostics-color=always",
    "-fopenmp",
    "-O3",
    "-g",
    "${fileDirname}\\*.cpp",
    "-o",
    "${fileDirname}\\${fileBasenameNoExtension}.exe"
],
``` 
Both two methods can change compiler optimizations and below are our benchmarking results.
| Function                | Size      | Compiler Optimization |  Execution Time |
|:-----------------------:|:---------:|:---------------------:|:------------------:|
| <b>multiply_mv_row_major</b>   | 2×2       | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 4×4       | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 64x64     | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 128x128   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 256x256   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 512x512   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 1024x1024 | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 2048x2048 | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 4096x4096 | O0                    | ms                 |
|    |           | O3                    | ms                 |
| <b>multiply_mv_col_major</b>   | 2×2       | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 4×4       | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 64x64     | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 128x128   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 256x256   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 512x512   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 1024x1024 | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 2048x2048 | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 4096x4096 | O0                    | ms                 |
|    |           | O3                    | ms                 |
| <b>multiply_mm_naive</b>   | 2×2       | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 4×4       | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 64x64     | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 128x128   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 256x256   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 512x512   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 1024x1024 | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 2048x2048 | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 4096x4096 | O0                    | ms                 |
|    |           | O3                    | ms                 |
| <b>multiply_mm_transposed_b</b>   | 2×2       | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 4×4       | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 64x64     | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 128x128   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 256x256   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 512x512   | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 1024x1024 | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 2048x2048 | O0                    | ms                 |
|    |           | O3                    | ms                 |
|    | 4096x4096 | O0                    | ms                 |
|    |           | O3                    | ms                 |

It can be observed that for smaller matrices, the effect of compiler optimization is not significant; the execution time of O0 may even be shorter than that of O3. However, as the size increases, the effect of compiler optimization becomes more pronounced, and the execution of O3 is much lower than that of O0 for large sizes.

When short and frequently called functions are inlined, execution efficiency can be significantly improved. However, inlining complex and lengthy functions may instead lead to slower performance. From the perspective of assembly language, if inline is not used, the compiler will generate a call instruction to invoke the function and a ret instruction at the end to return the result. Both call and ret introduce additional overhead. Therefore, for short and frequently called functions, repeated use of call and ret can greatly increase execution time. By using inline, the small function body is directly substituted into the assembly code, eliminating the overhead of call and ret and thus improving efficiency. On the other hand, for complex and lengthy functions, inlining may cause code size expansion, potentially slowing down instruction caching and leading to reduced performance.

$5.$ Profiling

TODO

$6.$ Optimization Strategies

Our optimization strategy mainly consists of the following three points. 
1. Compiler Optimization
   
   We observed that for most matrix multiplication operations, using O3 compiler optimization performs significantly better than O0. Therefore, for optimization purposes, we use O3 and modify the `args` parameter in tasks.json accordingly. 
2. Parallelization
   
   We found that during the execution of the code, the CPU usage was only about 10%, and the computational resources were not being fully utilized. Therefore, we use `#pragma omp parallel for` to implement parallel operations simply to improve computation speed. 
3. Blocking
   
   TODO

Benchmark our optimized version against the baseline, the results are displayed as table below.
Both two methods can change compiler optimizations and below are our benchmarking results.
| Function                | Size      | Baseline / Optimized |  Execution Time |
|:-----------------------:|:---------:|:---------------------:|:------------------:|
| <b>multiply_mv_row_major</b>   | 2×2       | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 4×4       | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 64x64     | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 128x128   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 256x256   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 512x512   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 1024x1024 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 2048x2048 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 4096x4096 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
| <b>multiply_mv_col_major</b>   | 2×2       | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 4×4       | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 64x64     | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 128x128   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 256x256   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 512x512   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 1024x1024 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 2048x2048 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 4096x4096 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
| <b>multiply_mm_naive</b>   | 2×2       | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 4×4       | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 64x64     | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 128x128   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 256x256   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 512x512   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 1024x1024 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 2048x2048 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 4096x4096 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
| <b>multiply_mm_transposed_b</b>   | 2×2       | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 4×4       | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 64x64     | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 128x128   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 256x256   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 512x512   | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 1024x1024 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 2048x2048 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |
|    | 4096x4096 | Baseline                    | ms                 |
|    |           | Optimized                    | ms                 |