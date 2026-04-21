#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include <omp.h>


#define sgemm_plain 1
#define sgemm_improved 2
#define sgemm_openblas 3

struct Matrix {
    size_t rows;
    size_t cols;
    size_t ld; // leading dimension for BLAS
    float *data;
};

struct Matrix* generate_matrix(size_t rows, size_t cols);
void freeMatrix(struct Matrix *mat);
void check_result(struct Matrix *C1, struct Matrix *C2, struct Matrix *C3, size_t rows, size_t cols);

void matmul_plain(int rows1, int cols1, int cols2, const float *a, const float *b, float *c);
void matmul_improved(int rows1, int cols1, int cols2, const float *a, const float *b, float *c);
void sgemm_OpenBLAS(struct Matrix *A, struct Matrix *B, struct Matrix *C);
void sgemm(struct Matrix *A, struct Matrix *B, struct Matrix *C, int type);