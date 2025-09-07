#include <iostream>
#include <chrono>
#include "Original_Linear_Operation.h"

int main() {
    // Instance for Matrix-Vector operations
    const int rows1 = 1000;
    const int cols1 = 1000;
    double* matrix = new double[rows1 * cols1];
    double* vector = new double[cols1];
    double* result_row_major = new double[rows1];
    double* result_col_major = new double[cols1];

    // Instance for Matrix-Matrix operations
    const int rowsA = 1000;
    const int colsA = 1000;
    const int rowsB = colsA; 
    const int colsB = 1000;
    double* matrixA = new double[rowsA * colsA];
    double* matrixB = new double[rowsB * colsB];
    double* result_matrix = new double[rowsA * colsB];



    

    // Release resources
    delete[] matrix;
    delete[] vector;
    delete[] result_row_major;
    delete[] result_col_major;
    delete[] matrixA;
    delete[] matrixB;
    delete[] result_matrix;

    return 0;
}