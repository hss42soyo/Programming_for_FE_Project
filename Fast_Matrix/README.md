# Fast Matrix

$1.$ Analyze the Basic Implementation: Understand why the provided sumMatrixBasic function might not be the most efficient. Consider function call overhead and memory access patterns.

1. The sumMatrixBasic function calls the ```add``` function ```getElement``` function, but not inline, which results in a lot of time overhead.

2. The unoptimized version uses ```getElement``` function(```matrix[i][j]```) instead of a pointer to access the elements of the matrix row, which reduces cache locality, due to uncontinuous memory access.

$2.$ Implement an Optimized Version: Create a new function (e.g., sumMatrixOptimized) that calculates the sum of the matrix elements using the optimization techniques mentioned (inlining, cache locality, pointer manipulation).

Shown as below:
```cpp
long long sumMatrixBasic_Optimized(const std::vector<std::vector<int>>& matrix) {
    long long sum = 0;

    for (int i = 0; i < SIZE; ++i) {
        const int* curr_row = matrix[i].data();
        for (int j = 0; j < SIZE; ++j) {
            sum = sum + curr_row[j];
        }
    }
    return sum;
}
```

$3.$ Measure Execution Time: Use std::chrono to measure the execution time of both the basic and their optimized implementations for the given SIZE.

The optimized version is around 3 times better than unoptimized version.

Basic Sum: 214122
Basic Time: 81 milliseconds
Optimized Sum: 214122
Optimized Time: 24 milliseconds

$4.$ Meet the Target Performance: The goal is to reduce the execution time of the optimized version significantly compared to the basic version and go under a specific target execution time (you will define this target based on your testing environment – for example, "under 50 milliseconds").

Our goal is to be under 40 milliseconds, and the real execution time is 24 milliseconds, which is under the goal.

$5.$ Explain Optimizations: In comments within their code or in a separate document, students should explain the optimizations they implemented and why they believe these optimizations improved performance.

The optimizations implemented in the optimized version of the sumMatrix function include:

   1. Inlining: The sumMatrixBasic function calls the sumMatrixHelper function, which adds overhead due to function call. In the optimized version, the sumMatrixHelper function is inlined, which eliminates the function call overhead.

   2. Cache Locality: The optimized version uses a pointer to access the elements of the matrix row, which improves cache locality. By accessing the elements in a contiguous block of memory, the processor can take advantage of cache lines and reduce the number of cache misses.
   
   3. Compiler Optimizations: The compiler (-o2, -o3) can perform various optimizations on the code, such as loop unrolling, constant propagation, and dead code elimination. We use ```-O3``` and the result turned out to be good.