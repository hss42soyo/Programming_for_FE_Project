# High Performance Linear Algebra Kernels

## Team Members

Yunxuan Chen

Shengsheng He

Chen Ye

(Tianyue Tang is in our group but this project will be done by himself.)

## Discussion Questions
$1.$ Explain the key differences between pointers and references in C++. When would you choose to use a pointer over a reference, and vice versa, in the context of implementing numerical algorithms?

Differences:
1. Memory Space: Using references doesn't require allocating additional memory space, but pointers do.
2. Initialization: Pointers can be initialized to `nullptr` or any valid memory address, whereas references must be initialized and cannot be set to `nullptr` or changed to refer to another object after initialization.
3. Assignment: Pointers should be deleted after being used and can be reassigned to point to different objects, whereas references cannot be reassigned to refer to different objects after initialization.
4. Dereferencing: Pointers store the address pointed to and must be dereferenced using the `*` operator to access the object they point to, whereas references copies the address and can be used directly without the `*` operator. 

Suitable Use Cases for References:

1. Parameter Passing: If a function needs to modify a parameter but doesn't want to explicitly write a pointer (e.g., `foo(double& x)`), references are more intuitive and avoid the syntactic noise of `*` and `->`.

2. Operating Small Objects or Scalars: For example, passing matrix dimensions or scalar parameters.

3. Avoiding Null Pointer Risks: References guarantee that the object is bound and are suitable for parameters that must exist.

Suitable Use Cases for Pointers

1. Dynamic Memory Management: For example, allocating matrices or vectors on the heap.

2. Optional Parameters/Conditional Data: If the object may not exist, using nullptr is more natural than using a reference.

3. Data Protection: Pointers can be declared const to avoid data modification and read errors.

4. Processing Arrays or Large Blocks of Data: Pointer arithmetic is efficient (e.g., iterating over a vector with double* data, combined with SIMD/vectorization optimizations).

$2.$ How does the row-major and column-major storage order of matrices affect memory access patterns and cache locality during matrix-vector and matrix-matrix multiplication? Provide specific examples from your implementations and benchmarking results.

The storage format of a row-major matrix in memory is that the data in a row is stored continuously, while the data in a column is not stored continuously. However, a column-major matrix is the opposite. In addition, during a data access, the cache will store continuous data in the cache due to spatial locality to improve efficiency. Therefore, in MV multiplication, since continuous row data is used for calculations, the row-major matrix runs faster. On the contrary, in MM multiplication, given a row-major matrix A, matrix B needs to use continuous column data for calculations, so using the transposed B matrix (column-major matrix) performs better. The specific data is as follows:

```
Testing Size: 4096x4096
Row-Major MV Multiplication Time: 50.255 ms
Col-Major MV Multiplication Time: 173.749 ms
Naive MM Multiplication Time: 883700 ms
MM Transposed B Multiplication Time: 626993 ms
```

$3.$Describe how CPU caches work (L1, L2, L3) and explain the concepts of temporal and spatial locality. How did you try to exploit these concepts in your optimizations?

L1 Cache is located closest to the CPU cores, it has a small capacity (tens of KB) and the fastest speed. L2 Cache is larger than L1 (hundreds of KB), but slower than L1. L3 Cache is shared between cores, has a large capacity (several to tens of MB), but it has the highest latency, but is still faster than memory. The data flow path is roughly: CPU → L1 → L2 → L3 → memory. CPU will go through L1 first, and if L1 misses, then it will go through L2, and so on until memory. 

Temporal locality: Recently accessed data is likely to be accessed again within a short period of time.

Spatial locality: Once a certain address is accessed, nearby addresses are likely to be accessed as well. This is because memory data is loaded in cache lines.

In our code, we leverage spatial locality and access memory sequentially to avoid large stride lengths. For example, in MV multiplication we use row-major matrix and in MM multiplication we transposed matrix B into a column-major matrix to improve spatial locality.

```
Testing Size: 4096x4096
Row-Major MV Multiplication Time: 50.255 ms
Col-Major MV Multiplication Time: 173.749 ms
Naive MM Multiplication Time: 883700 ms
MM Transposed B Multiplication Time: 626993 ms
```

$4.$What is memory alignment, and why is it important for performance? Did you observe a significant performance difference between aligned and unaligned memory in your experiments? Explain your findings.

Memory alignment means that the starting address of data in memory is a multiple of its type size. This allows the CPU to read data at a "natural boundary" all at once, rather than breaking it up into multiple smaller reads.

In our code, we use 64 bytes to do the align because the object is double type(8 bytes), but we don't see a significant performance difference between aligned and unaligned memory in our experiments.

TODO

$5.$Discuss the role of compiler optimizations (like inlining) in achieving high performance. How did the optimization level affect the performance of your baseline and optimized implementations? What are the potential drawbacks of aggressive optimization?

Compiler optimizations can help improve performance by reducing the overhead of function calls, eliminating unnecessary computations, and optimizing memory access patterns. Inlining, for example, normal funtions will be turned into call, ret and jump in assembly code. It will cost more time if not using inlining.

In our code, the improvement in baseline is more significant, but the improvement in baseline is less significant.

However, aggressive optimization can also lead to code bloat, making it harder to debug and maintain, and can also introduce subtle bugs.

$6.$Based on your profiling experience, what were the main performance bottlenecks in your initial implementations? How did your profiling results guide your optimization efforts?

The main performance bottlenecks in our initial implementations were cache misses and unparallel code execution. We use spatial locality to improve cache performance and use parallel code to improve code execution performance.

$7.$Reflect on the teamwork aspect of this assignment. How did dividing the initial implementation tasks and then collaborating on analysis and optimization work? What were the challenges and benefits of this approach?

One of our member, Shengsheng He, is very enthusastic about this assignment, and had done a part of code framework before we started. Based on his code, we first divided the initial implementation tasks, and then have a meeting offline to collaborate on analysis and optimization work, and finally we upload our code and do the merge. This approach can make us more efficient and save time.