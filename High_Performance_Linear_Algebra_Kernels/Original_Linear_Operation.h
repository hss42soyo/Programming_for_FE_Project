class OriginalLinearOperation {
public:
    // Matrix-Vector multiplication (Row-Major)
   void multiply_mv_row_major(const double* matrix, int rows, int cols,
                             const double* vector, double* result);
    // Matrix-Vector multiplication (Column-Major)
   void multiply_mv_col_major(const double* matrix, int rows, int cols,
                             const double* vector, double* result);
    // Matrix-Matrix multiplication (Naive)
   void multiply_mm_naive(const double* matrixA, int rowsA, int colsA,
                         const double* matrixB, int rowsB, int colsB,
                         double* result);
    // Matrix-Matrix multiplication (Transposed B)
   void multiply_mm_transposed(const double* matrixA, int rowsA, int colsA,
                               const double* matrixB_transposed, int rowsB, int colsB,
                               double* result);
};

