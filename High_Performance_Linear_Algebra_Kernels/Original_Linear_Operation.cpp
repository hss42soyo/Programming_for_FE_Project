#include <iostream>
#include "Original_Linear_Operation.h"

void OriginalLinearOperation::Verify(int cols,int rows){
    if (cols != rows){
        std::cout<<"Error: Matrix dimensions do not match!"<<std::endl;
        exit(1);
    }
}

void OriginalLinearOperation::Verify(int rowsA,int colsA,int rowsB,int colsB){
    if (colsA != rowsB){
        std::cout<<"Error: Matrix dimensions do not match!"<<std::endl;
        exit(1);
    }
}

void OriginalLinearOperation::multiply_mv_row_major(const double* matrix, int rows, int cols,
                                                  const double* vector, double* result) {
    for (int i = 0; i < rows; ++i) {
        result[i] = 0.0;
        for (int j = 0; j < cols; ++j) {
            result[i] += matrix[i * cols + j] * vector[j];
        }
    }
}

void OriginalLinearOperation::multiply_mv_col_major(const double* matrix, int rows, int cols,
                                                  const double* vector, double* result) {
    for (int i = 0; i < rows; ++i) {
        result[i] = 0.0;
        for (int j = 0; j < cols; ++j) {
            result[i] += matrix[j * rows + i] * vector[j];
        }
    }
}

void OriginalLinearOperation::multiply_mm_naive(const double* matrixA, int rowsA, int colsA,
                                               const double* matrixB, int rowsB, int colsB,
                                               double* result) {
    for (int i = 0; i < rowsA; ++i) {
        for (int j = 0; j < colsB; ++j) {
            result[i * colsB + j] = 0.0;
            for (int k = 0; k < colsA; ++k) {
                result[i * colsB + j] += matrixA[i * colsA + k] * matrixB[k * colsB + j];
            }
        }
    }
}

void OriginalLinearOperation::multiply_mm_transposed_b(const double* matrixA, int rowsA, int colsA,
                                                    const double* matrixB_transposed, int rowsB, int colsB, double* result) {
    for (int i = 0; i < rowsA; ++i) {
        const double* arow = matrixA + i * colsA;  // i th row start pointer of A
        double* crow = result + i * rowsB;         // i th row start pointer of result
        for (int j = 0; j < rowsB; ++j) {
            double sum = 0.0;
            // C[i,j] = sum_k A[i,k] * (B^T)[k,j]
            for (int k = 0; k < colsA; ++k) {
                sum += arow[k] * matrixB_transposed[k * rowsB + j];
            }
            crow[j] = sum;
        }
    }
}