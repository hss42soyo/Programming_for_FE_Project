#include <iostream>
#include <chrono>
#include <random>
#include "Original_Linear_Operation.h"

// Size: small: 64x64, medium: 256x256, large: 1024x1024
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

// Change these constants to test different sizes
// const int MATRIXSIZEROW = static_cast<int>(MatrixSize::TINY);
// const int MATRIXSIZECOL = static_cast<int>(MatrixSize::TINT2);
// const int VECTORSIZE = static_cast<int>(MatrixSize::TINT2);

// const int ROWSIZEA = static_cast<int>(MatrixSize::TINY);
// const int COLSIZEA = static_cast<int>(MatrixSize::TINY);
// const int ROWSIZEB = static_cast<int>(MatrixSize::TINY);
// const int COLSIZEB = static_cast<int>(MatrixSize::TINY);

// function to toggle sizes
void set_sizes(MatrixSize size, int &matrix_rows, int &matrix_cols, int &vector_size,
               int &rowsA, int &colsA, int &rowsB, int &colsB) {
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


void CreateRandomMatrix_RowMajor(double* matrix, int rows, int cols) {
    std::random_device rd;
    std::mt19937 gen(SEED_MV_M);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < rows * cols; ++i) {
        matrix[i] = static_cast<double>(distrib(gen));
    }
}
void CreateRandomMatrix_ColMajor(double* matrix, int rows, int cols) {
    std::random_device rd;
    std::mt19937 gen(SEED_MV_M);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[j * rows + i] = static_cast<double>(distrib(gen));
        }
    }
}
void CreateRandomVector(double* vector, int size) {
    std::random_device rd;
    std::mt19937 gen(SEED_MV_M);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < size; ++i) {
        vector[i] = static_cast<double>(distrib(gen));
    }
}

void CreateRandomMatrix_RowMajor_Seed(double* matrix, int rows, int cols, int seed) {
    std::random_device rd;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < rows * cols; ++i) {
        matrix[i] = static_cast<double>(distrib(gen));
    }
}

void CreateRandomMatrix_ColMajor_Seed(double* matrix, int rows, int cols, int seed) {
    std::random_device rd;
    std::mt19937 gen(seed);
    std::uniform_int_distribution<> distrib(MIN, MAX);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[j * rows + i] = static_cast<double>(distrib(gen));
        }
    }
}

int main() {
    OriginalLinearOperation originalOp;
<<<<<<< HEAD
    // Set sizes
    int MATRIXSIZEROW, MATRIXSIZECOL, VECTORSIZE, ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB;
    set_sizes(MatrixSize::LARGE, MATRIXSIZEROW, MATRIXSIZECOL, VECTORSIZE, ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);
    // Initialize matrices and vectors
    double* matrix_row = new double[MATRIXSIZEROW * MATRIXSIZECOL];
    double* matrix_col = new double[MATRIXSIZECOL * MATRIXSIZEROW];
    double* vector = new double[VECTORSIZE];
    double* result_row_major = new double[VECTORSIZE];
    double* result_col_major = new double[VECTORSIZE];
=======
    int MATRIXSIZEROW, MATRIXSIZECOL, VECTORSIZE, ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB;

    for(auto Size : {MatrixSize::TINY, MatrixSize::TINT2, 
        MatrixSize::SMALL1, MatrixSize::SMALL2, MatrixSize::MEDIUM, 
        MatrixSize::MEDIUM2, MatrixSize::LARGE, MatrixSize::LARGE2,
        MatrixSize::ENORMOUS}) {
>>>>>>> origin/Test

        std::cout << "Testing Size: " << static_cast<int>(Size) << "x" << static_cast<int>(Size) << std::endl;
        // Set sizes
        set_sizes(Size, MATRIXSIZEROW, MATRIXSIZECOL, VECTORSIZE, ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);
        // Initialize matrices and vectors
        double* matrix_row = new double[MATRIXSIZEROW * MATRIXSIZECOL];
        double* matrix_col = new double[MATRIXSIZECOL * MATRIXSIZEROW];
        double* vector = new double[VECTORSIZE];
        double* result_row_major = new double[VECTORSIZE];
        double* result_col_major = new double[VECTORSIZE];
        double* result_row_major_opt = new double[VECTORSIZE];
        double* result_col_major_opt = new double[VECTORSIZE];

        // Create random Matrix and Vector
        CreateRandomMatrix_RowMajor(matrix_row, MATRIXSIZEROW, MATRIXSIZECOL);
        CreateRandomMatrix_ColMajor(matrix_col, MATRIXSIZEROW, MATRIXSIZECOL);
        CreateRandomVector(vector, VECTORSIZE);

        // Verify dimensions for MV multiplication
        originalOp.Verify(MATRIXSIZECOL,VECTORSIZE);

        // Calculate the result and measure time for Row-Major MV multiplication
        auto start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mv_row_major(matrix_row, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_row_major);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_row_major = end - start;

        // std::cout << "Result (Row-Major MV): " << std::endl;
        // for (int i = 0; i < MATRIXSIZEROW; ++i) {
        //     std::cout << result_row_major[i] << " " << std::endl;
        // }
        std::cout << "Row-Major MV Multiplication Time: " << duration_row_major.count() << " ms" << std::endl;
        
        // Optimized Row-Major MV multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mv_row_major_opt(matrix_row, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_row_major_opt);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_row_major_opt = end - start;
        std::cout << "Row-Major MV Multiplication (Optimized) Time: " << duration_row_major_opt.count() << " ms" << std::endl;

        // Calculate the result and measure time for Col-Major MV multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mv_col_major(matrix_col, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_col_major);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_col_major = end - start;

        // std::cout << "Result (Col-Major MV): " << std::endl;
        // for (int i = 0; i < MATRIXSIZEROW; ++i) {
        //     std::cout << result_col_major[i] << " " << std::endl;
        // }

        std::cout << "Col-Major MV Multiplication Time: " << duration_col_major.count() << " ms" << std::endl;
        
        // Optimized Col-Major MV multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mv_col_major_opt(matrix_col, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_col_major_opt);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_col_major_opt = end - start;
        std::cout << "Col-Major MV Multiplication (Optimized) Time: " << duration_col_major_opt.count() << " ms" << std::endl;

        // Instance for Matrix-Matrix operations

        double* matrixA = new double[ROWSIZEA * COLSIZEA];
        double* matrixB = new double[ROWSIZEB * COLSIZEB];
        double* transposed_matrixB = new double[ROWSIZEB * COLSIZEB];
        double* result_matrix = new double[ROWSIZEA * COLSIZEB];

        // Create random matrices
        CreateRandomMatrix_RowMajor_Seed(matrixA, ROWSIZEA, COLSIZEA, SEED_MM_A);
        CreateRandomMatrix_RowMajor_Seed(matrixB, ROWSIZEB, COLSIZEB, SEED_MM_B);
        CreateRandomMatrix_ColMajor_Seed(transposed_matrixB, ROWSIZEB, COLSIZEB, SEED_MM_B); // For transposed multiplication

        // Verify dimensions for MM multiplication
        originalOp.Verify(ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

        // Calculate the result and measure time for native MM multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mm_naive(matrixA, ROWSIZEA, COLSIZEA, matrixB, ROWSIZEB, COLSIZEB, result_matrix);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_mm_naive = end - start;
        std::cout << "Naive MM Multiplication Time: " << duration_mm_naive.count() << " ms" << std::endl;

        // Optimized Naive MM multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mm_naive_opt(matrixA, ROWSIZEA, COLSIZEA,
                            matrixB, ROWSIZEB, COLSIZEB, result_matrix);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_mm_naive_opt = end - start;
        std::cout << "Optimized Naive MM Multiplication Time: " << duration_mm_naive_opt.count() << " ms" << std::endl;

        // Calculate the result and measure time for Transposed MM multiplication

        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mm_transposed_b(matrixA, ROWSIZEA, COLSIZEA, transposed_matrixB, ROWSIZEB, COLSIZEB, result_matrix);
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration_mm_transposed_b = end - start;
        // std::cout << "Result (MM Transposed B): " << std::endl;
        // for (int i = 0; i < MATRIXSIZEROW; ++i) {
        //     for (int j = 0; j < MATRIXSIZECOL; ++j) {
        //         std::cout << result_matrix[i * MATRIXSIZECOL + j] << " ";
        //     }
        //     std::cout << std::endl;
        // }
        std::cout << "MM Transposed B Multiplication Time: " << duration_mm_transposed_b.count() << " ms" << std::endl;

        // Optimized Transposed MM multiplication
        start = std::chrono::high_resolution_clock::now();
        originalOp.multiply_mm_transposed_b_opt(matrixA, ROWSIZEA, COLSIZEA,
                               transposed_matrixB, ROWSIZEB, COLSIZEB, result_matrix);
        end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> duration_mm_transposed_b_opt = end - start;
        std::cout << "Optimized MM Transposed B Multiplication Time: " << duration_mm_transposed_b_opt.count() << " ms" << std::endl;

        // Release resources
        delete[] matrix_row;
        delete[] matrix_col;
        delete[] result_row_major_opt;
        delete[] result_col_major_opt;
        delete[] vector;
        delete[] result_row_major;
        delete[] result_col_major;
        delete[] matrixA;
        delete[] matrixB;
        delete[] transposed_matrixB;
        delete[] result_matrix;

        std::cout << "----------------------------------------" << std::endl;

    }
<<<<<<< HEAD
    std::cout << "Row-Major MV Multiplication Time: " << duration_row_major.count() << " ms" << std::endl;



    // Calculate the result and measure time for Col-Major MV multiplication
    start = std::chrono::high_resolution_clock::now();
    originalOp.multiply_mv_col_major(matrix_col, MATRIXSIZEROW, MATRIXSIZECOL, vector, result_col_major);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_col_major = end - start;
    std::cout << "Result (Col-Major MV): " << std::endl;
    for (int i = 0; i < MATRIXSIZEROW; ++i) {
        //std::cout << result_col_major[i] << " " << std::endl;
    }
    std::cout << "Col-Major MV Multiplication Time: " << duration_col_major.count() << " ms" << std::endl;

    // Instance for Matrix-Matrix operations

    double* matrixA = new double[ROWSIZEA * COLSIZEA];
    double* matrixB = new double[ROWSIZEB * COLSIZEB];
    double* result_matrix = new double[ROWSIZEA * COLSIZEB];

    // Create random matrices
    CreateRandomMatrix_RowMajor_Seed(matrixA, ROWSIZEA, COLSIZEA, SEED_MM_A);
    CreateRandomMatrix_RowMajor_Seed(matrixB, ROWSIZEB, COLSIZEB, SEED_MM_B);

    // Verify dimensions for MM multiplication
    originalOp.Verify(ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

    // Calculate the result and measure time for native MM multiplication
    start = std::chrono::high_resolution_clock::now();
    originalOp.multiply_mm_naive(matrixA, ROWSIZEA, COLSIZEA, matrixB, ROWSIZEB, COLSIZEB, result_matrix);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_mm_naive = end - start;
    std::cout << "Naive MM Multiplication Time: " << duration_mm_naive.count() << " ms" << std::endl;

    // Calculate the result and measure time for Transposed MM multiplication
    // First, create the transposed matrixB

    double* matrixA_trans = new double[ROWSIZEA * COLSIZEA];
    double* matrixB_trans = new double[ROWSIZEB * COLSIZEB];
    double* result_matrix_trans = new double[ROWSIZEA * COLSIZEB];

    // Create random matrices
    CreateRandomMatrix_RowMajor_Seed(matrixA_trans, ROWSIZEA, COLSIZEA, SEED_MM_A);
    CreateRandomMatrix_RowMajor_Seed(matrixB_trans, ROWSIZEB, COLSIZEB, SEED_MM_B);

    // Verify dimensions for MM multiplication
    originalOp.Verify(ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

    // Calculate the result and measure time for native MM multiplication
    start = std::chrono::high_resolution_clock::now();
    originalOp.multiply_mm_transposed_b(matrixA_trans, ROWSIZEA, COLSIZEA, matrixB_trans, ROWSIZEB, COLSIZEB, result_matrix_trans);
    end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_mm_transposed = end - start;
    std::cout << "Transposed MM Multiplication Time: " << duration_mm_transposed.count() << " ms" << std::endl;

    // Release resources
    delete[] matrix_row;
    delete[] matrix_col;
    delete[] vector;
    delete[] result_row_major;
    delete[] result_col_major;
    delete[] matrixA;
    delete[] matrixB;
    delete[] result_matrix;
=======
>>>>>>> origin/Test

    return 0;
}
