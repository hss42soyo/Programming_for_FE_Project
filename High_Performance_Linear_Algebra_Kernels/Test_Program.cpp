#include <iostream>
#include <chrono>
#include <random>
#include "Original_Linear_Operation.h"

// Size: small: 64x64, medium: 256x256, large: 1024x1024
enum class MatrixSize {
    TINY = 2,
    SMALL = 64, 
    MEDIUM = 256, 
    LARGE = 1024
};

// Change these constants to test different sizes
const int MATRIXSIZE = static_cast<int>(MatrixSize::TINY);
const int VECTORSIZE = static_cast<int>(MatrixSize::TINY);
const int MATRIXSIZEA = static_cast<int>(MatrixSize::TINY);
const int MATRIXSIZEB = static_cast<int>(MatrixSize::TINY);

 std::random_device rd;
 std::mt19937 gen(rd());
 std::uniform_int_distribution<> distrib(-100, 100);

void CreateRandomMatrix_RowMajor(double* matrix, int rows, int cols) {
    for (int i = 0; i < rows * cols; ++i) {
        matrix[i] = static_cast<double>(distrib(gen));
        //matrix[i] = i + 1;
    }
}
void CreateRandomMatrix_ColMajor(double* matrix, int rows, int cols) {
    for (int j = 0; j < cols; ++j) {
        for (int i = 0; i < rows; ++i) {
            matrix[i * cols + j] = static_cast<double>(distrib(gen));
        }
    }
}
void CreateRandomVector(double* vector, int size) {
    for (int i = 0; i < size; ++i) {
        vector[i] = static_cast<double>(distrib(gen));
        //vector[i] = i + 1;
    }
}

int main() {
    OriginalLinearOperation originalOp;
    // Initialize matrices and vectors
    double* matrix_row = new double[MATRIXSIZE * MATRIXSIZE];
    double* matrix_col = new double[MATRIXSIZE * MATRIXSIZE];
    double* vector = new double[VECTORSIZE];
    double* result_row_major = new double[MATRIXSIZE];
    double* result_col_major = new double[MATRIXSIZE];

    // Create random Matrix and Vector
    CreateRandomMatrix_RowMajor(matrix_row, MATRIXSIZE, MATRIXSIZE);
    CreateRandomMatrix_ColMajor(matrix_col, MATRIXSIZE, MATRIXSIZE);
    CreateRandomVector(vector, VECTORSIZE);

    // Verify dimensions for MV multiplication
    originalOp.Verify(MATRIXSIZE,MATRIXSIZE);

    auto start = std::chrono::high_resolution_clock::now();
    originalOp.multiply_mv_row_major(matrix_row, MATRIXSIZE, MATRIXSIZE, vector, result_row_major);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_row_major = end - start;
    std::cout << "Result (Row-Major MV): " << std::endl;
    for (int i = 0; i < VECTORSIZE; ++i) {
        std::cout << result_row_major[i] << " " << std::endl;
    }
    std::cout << "Row-Major MV Multiplication Time: " << duration_row_major.count() << " ms" << std::endl;

    // Instance for Matrix-Matrix operations
    const int rowsA = 1000;
    const int colsA = 1000;
    const int rowsB = colsA; 
    const int colsB = 1000;
    double* matrixA = new double[rowsA * colsA];
    double* matrixB = new double[rowsB * colsB];
    double* result_matrix = new double[rowsA * colsB];



    

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