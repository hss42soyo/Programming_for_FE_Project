#include <iostream>
#include <chrono>
#include <random>
#include "Original_Linear_Operation.h"

// Size: small: 64x64, medium: 256x256, large: 1024x1024
enum class MatrixSize {
    TINY = 2,
    TINT2 = 4,
    SMALL = 64, 
    MEDIUM = 256, 
    LARGE = 1024
};

// Change these constants to test different sizes
const int MATRIXSIZEROW = static_cast<int>(MatrixSize::TINY);
const int MATRIXSIZECOL = static_cast<int>(MatrixSize::TINT2);
const int VECTORSIZE = static_cast<int>(MatrixSize::TINT2);

const int ROWSIZEA = static_cast<int>(MatrixSize::TINY);
const int COLSIZEA = static_cast<int>(MatrixSize::TINY);
const int ROWSIZEB = static_cast<int>(MatrixSize::TINY);
const int COLSIZEB = static_cast<int>(MatrixSize::TINY);

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

    // Initialize matrices and vectors
    double* matrix_row = new double[MATRIXSIZEROW * MATRIXSIZECOL];
    double* matrix_col = new double[MATRIXSIZECOL * MATRIXSIZEROW];
    double* vector = new double[VECTORSIZE];
    double* result_row_major = new double[VECTORSIZE];
    double* result_col_major = new double[VECTORSIZE];

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
    std::cout << "Result (Row-Major MV): " << std::endl;
    for (int i = 0; i < MATRIXSIZEROW; ++i) {
        //std::cout << result_row_major[i] << " " << std::endl;
    }
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
    double* transposed_matrixB = new double[ROWSIZEB * COLSIZEB];
    double* result_matrix = new double[ROWSIZEA * COLSIZEB];

    // Create random matrices
    CreateRandomMatrix_RowMajor_Seed(matrixA, ROWSIZEA, COLSIZEA, SEED_MM_A);
    CreateRandomMatrix_RowMajor_Seed(matrixB, ROWSIZEB, COLSIZEB, SEED_MM_B);
    CreateRandomMatrix_ColMajor_Seed(transposed_matrixB, ROWSIZEB, COLSIZEB, SEED_MM_B); // For transposed multiplication

    // Verify dimensions for MM multiplication
    originalOp.Verify(ROWSIZEA, COLSIZEA, ROWSIZEB, COLSIZEB);

    // Calculate the result and measure time for native MM multiplication


    // Calculate the result and measure time for Transposed MM multiplication


    // Release resources
    delete[] matrix_row;
    delete[] matrix_col;
    delete[] vector;
    delete[] result_row_major;
    delete[] result_col_major;
    delete[] matrixA;
    delete[] matrixB;
    delete[] result_matrix;

    return 0;
}
