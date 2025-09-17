#include <iostream>
#include <chrono>
#include <random>
#include "Original_Linear_Operation.h"
#include <cstdlib>

// Size: small: 64x64, medium: 256x256, large: 1024x1024
enum class MatrixSize
{
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

// function to toggle sizes
void set_sizes(MatrixSize size, int &matrix_rows, int &matrix_cols, int &vector_size,
               int &rowsA, int &colsA, int &rowsB, int &colsB)
{
    matrix_rows = static_cast<int>(size);
    matrix_cols = static_cast<int>(size);
    vector_size = static_cast<int>(size);
    rowsA = static_cast<int>(size);
    colsA = static_cast<int>(size);
    rowsB = static_cast<int>(size);
    colsB = static_cast<int>(size);
}

// Random number generation
const int SEED_MV_M = 42;
const int SEED_MV_V = 42;
const int SEED_MM_A = 42;
const int SEED_MM_B = 42;
const int MIN = -100;
const int MAX = 100;

void CreateRandomMatrix_RowMajor(double *matrix, int rows, int cols)
{
    std::random_device rd;
    std::mt19937 gen(SEED_MV_M);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < rows * cols; ++i)
    {
        matrix[i] = static_cast<double>(distrib(gen));
    }
}
void CreateRandomMatrix_ColMajor(double *matrix, int rows, int cols)
{
    std::random_device rd;
    std::mt19937 gen(SEED_MV_M);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            matrix[j * rows + i] = static_cast<double>(distrib(gen));
        }
    }
}
void CreateRandomVector(double *vector, int size)
{
    std::random_device rd;
    std::mt19937 gen(SEED_MV_M);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < size; ++i)
    {
        vector[i] = static_cast<double>(distrib(gen));
    }
}

void CreateRandomMatrix_RowMajor_Seed(double *matrix, int rows, int cols, int seed)
{
    std::random_device rd;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < rows * cols; ++i)
    {
        matrix[i] = static_cast<double>(distrib(gen));
    }
}

int main()
{
    OriginalLinearOperation originalOp;
    // Set sizes
    int MATRIXSIZEROW, MATRIXSIZECOL, VECTORSIZE, ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB;
    std::cout << "Baseline" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (auto Size : {MatrixSize::TINY, MatrixSize::TINT2,
                      MatrixSize::SMALL1, MatrixSize::SMALL2,
                      MatrixSize::MEDIUM, MatrixSize::MEDIUM2,
                      MatrixSize::LARGE, MatrixSize::LARGE2,
                      MatrixSize::ENORMOUS})
    {

        std::cout << "Testing Size: " << static_cast<int>(Size) << "x" << static_cast<int>(Size) << std::endl;
        set_sizes(Size, MATRIXSIZEROW, MATRIXSIZECOL, VECTORSIZE, ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

        // unaligned matrices and vectors
        double *matrix_row = new double[MATRIXSIZEROW * MATRIXSIZECOL];
        double *matrix_col = new double[MATRIXSIZECOL * MATRIXSIZEROW];
        double *vector = new double[VECTORSIZE];
        double *result_row_major = new double[VECTORSIZE];
        double *result_col_major = new double[VECTORSIZE];

        double *matrixA = new double[ROWSIZEA * COLSIZEA];
        double *matrixB = new double[ROWSIZEB * COLSIZEB];
        double *result_matrix = new double[ROWSIZEA * COLSIZEB];

        double *matrixA_trans = new double[COLSIZEA * ROWSIZEA];
        double *matrixB_trans = new double[COLSIZEB * ROWSIZEB];
        double *result_matrix_trans = new double[COLSIZEA * COLSIZEB];

        // Create random Matrix and Vector
        CreateRandomMatrix_RowMajor(matrix_row, MATRIXSIZEROW, MATRIXSIZECOL);
        CreateRandomMatrix_ColMajor(matrix_col, MATRIXSIZEROW, MATRIXSIZECOL);
        CreateRandomVector(vector, VECTORSIZE);
        // Create random matrices
        CreateRandomMatrix_RowMajor_Seed(matrixA, ROWSIZEA, COLSIZEA, SEED_MM_A);
        CreateRandomMatrix_RowMajor_Seed(matrixB, ROWSIZEB, COLSIZEB, SEED_MM_B);
        // Create random matrices
        CreateRandomMatrix_RowMajor_Seed(matrixA_trans, ROWSIZEA, COLSIZEA, SEED_MM_A);
        CreateRandomMatrix_RowMajor_Seed(matrixB_trans, ROWSIZEB, COLSIZEB, SEED_MM_B);

        // Verify dimensions for MV multiplication
        originalOp.Verify(MATRIXSIZECOL, VECTORSIZE);

        // Calculate the result and measure time for Row-Major MV multiplication
        auto start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mv_row_major(matrix_row, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_row_major);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_row_major = end - start;
        std::cout << "Result (Row-Major MV): " << std::endl;
        for (int i = 0; i < MATRIXSIZEROW; ++i)
        {
            // std::cout << result_row_major[i] << " " << std::endl;
        }
        std::cout << "Row-Major MV Multiplication Time: " << duration_row_major.count() << " ms" << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        // Calculate the result and measure time for Col-Major MV multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mv_col_major(matrix_col, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_col_major);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_col_major = end - start;
        std::cout << "Result (Col-Major MV): " << std::endl;
        for (int i = 0; i < MATRIXSIZEROW; ++i)
        {
            // std::cout << result_col_major[i] << " " << std::endl;
        }
        std::cout << "Col-Major MV Multiplication Time: " << duration_col_major.count() << " ms" << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        // Instance for Matrix-Matrix operations
        // Verify dimensions for MM multiplication
        originalOp.Verify(ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

        // Calculate the result and measure time for native MM multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mm_naive(matrixA, ROWSIZEA, COLSIZEA, matrixB, ROWSIZEB, COLSIZEB, result_matrix);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_mm_naive = end - start;
        std::cout << "Naive MM Multiplication Time: " << duration_mm_naive.count() << " ms" << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        // Calculate the result and measure time for Transposed MM multiplication
        // Verify dimensions for MM multiplication
        originalOp.Verify(ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

        // Calculate the result and measure time for native MM multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mm_transposed_b(matrixA_trans, ROWSIZEA, COLSIZEA, matrixB_trans, ROWSIZEB, COLSIZEB, result_matrix_trans);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_mm_transposed = end - start;
        std::cout << "Transposed MM Multiplication Time: " << duration_mm_transposed.count() << " ms" << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        // Release resources
        free(matrix_row);
        free(matrix_col);
        free(vector);
        free(result_row_major);
        free(result_col_major);
        free(matrixA);
        free(matrixB);
        free(result_matrix);
        free(matrixA_trans);
        free(matrixB_trans);
        free(result_matrix_trans);
    }

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Memory Alignment" << std::endl;
    for (auto Size : {MatrixSize::TINY, MatrixSize::TINT2,
                      MatrixSize::SMALL1, MatrixSize::SMALL2,
                      MatrixSize::MEDIUM, MatrixSize::MEDIUM2,
                      MatrixSize::LARGE, MatrixSize::LARGE2,
                      MatrixSize::ENORMOUS})
    {

        std::cout << "Testing Size: " << static_cast<int>(Size) << "x" << static_cast<int>(Size) << std::endl;
        set_sizes(Size, MATRIXSIZEROW, MATRIXSIZECOL, VECTORSIZE, ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

        // Aligned matrices and vectors
        double *matrix_row;
        posix_memalign((void **)&matrix_row, 64, MATRIXSIZEROW * MATRIXSIZECOL * sizeof(double));
        double *matrix_col;
        posix_memalign((void **)&matrix_col, 64, MATRIXSIZECOL * MATRIXSIZEROW * sizeof(double));
        double *vector;
        posix_memalign((void **)&vector, 64, VECTORSIZE * sizeof(double));
        double *result_row_major;
        posix_memalign((void **)&result_row_major, 64, VECTORSIZE * sizeof(double));
        double *result_col_major;
        posix_memalign((void **)&result_col_major, 64, VECTORSIZE * sizeof(double));

        double *matrixA;
        posix_memalign((void **)&matrixA, 64, ROWSIZEA * COLSIZEA * sizeof(double));
        double *matrixB;
        posix_memalign((void **)&matrixB, 64, ROWSIZEB * COLSIZEB * sizeof(double));
        double *result_matrix;
        posix_memalign((void **)&result_matrix, 64, ROWSIZEA * COLSIZEB * sizeof(double));

        double *matrixA_trans;
        posix_memalign((void **)&matrixA_trans, 64, ROWSIZEA * COLSIZEA * sizeof(double));
        double *matrixB_trans;
        posix_memalign((void **)&matrixB_trans, 64, ROWSIZEB * COLSIZEB * sizeof(double));
        double *result_matrix_trans;
        posix_memalign((void **)&result_matrix_trans, 64, ROWSIZEA * COLSIZEB * sizeof(double));

        // Create random Matrix and Vector
        CreateRandomMatrix_RowMajor(matrix_row, MATRIXSIZEROW, MATRIXSIZECOL);
        CreateRandomMatrix_ColMajor(matrix_col, MATRIXSIZEROW, MATRIXSIZECOL);
        CreateRandomVector(vector, VECTORSIZE);
        // Create random matrices
        CreateRandomMatrix_RowMajor_Seed(matrixA, ROWSIZEA, COLSIZEA, SEED_MM_A);
        CreateRandomMatrix_RowMajor_Seed(matrixB, ROWSIZEB, COLSIZEB, SEED_MM_B);
        // Create random matrices
        CreateRandomMatrix_RowMajor_Seed(matrixA_trans, ROWSIZEA, COLSIZEA, SEED_MM_A);
        CreateRandomMatrix_RowMajor_Seed(matrixB_trans, ROWSIZEB, COLSIZEB, SEED_MM_B);

        // Verify dimensions for MV multiplication
        originalOp.Verify(MATRIXSIZECOL, VECTORSIZE);

        // Calculate the result and measure time for Row-Major MV multiplication
        auto start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mv_row_major(matrix_row, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_row_major);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_row_major = end - start;
        std::cout << "Result (Row-Major MV): " << std::endl;
        for (int i = 0; i < MATRIXSIZEROW; ++i)
        {
            // std::cout << result_row_major[i] << " " << std::endl;
        }
        std::cout << "Row-Major MV Multiplication Time: " << duration_row_major.count() << " ms" << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        // Calculate the result and measure time for Col-Major MV multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mv_col_major(matrix_col, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_col_major);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_col_major = end - start;
        std::cout << "Result (Col-Major MV): " << std::endl;
        for (int i = 0; i < MATRIXSIZEROW; ++i)
        {
            // std::cout << result_col_major[i] << " " << std::endl;
        }
        std::cout << "Col-Major MV Multiplication Time: " << duration_col_major.count() << " ms" << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        // Instance for Matrix-Matrix operations
        // Verify dimensions for MM multiplication
        originalOp.Verify(ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

        // Calculate the result and measure time for native MM multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mm_naive(matrixA, ROWSIZEA, COLSIZEA, matrixB, ROWSIZEB, COLSIZEB, result_matrix);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_mm_naive = end - start;
        std::cout << "Naive MM Multiplication Time: " << duration_mm_naive.count() << " ms" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        // Calculate the result and measure time for Transposed MM multiplication
        // Verify dimensions for MM multiplication
        originalOp.Verify(ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

        // Calculate the result and measure time for native MM multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mm_transposed_b(matrixA_trans, ROWSIZEA, COLSIZEA, matrixB_trans, ROWSIZEB, COLSIZEB, result_matrix_trans);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_mm_transposed = end - start;
        std::cout << "Transposed MM Multiplication Time: " << duration_mm_transposed.count() << " ms" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        // Release resources
        free(matrix_row);
        free(matrix_col);
        free(vector);
        free(result_row_major);
        free(result_col_major);
        free(matrixA);
        free(matrixB);
        free(result_matrix);
        free(matrixA_trans);
        free(matrixB_trans);
        free(result_matrix_trans);
    }

    return 0;
}
