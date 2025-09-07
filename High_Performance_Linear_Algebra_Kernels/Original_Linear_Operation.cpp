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