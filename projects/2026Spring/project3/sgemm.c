#include "sgemm.h"

void matmul_plain(int rows1, int cols1, int cols2, const float *a, const float *b, float *c) {
    if(rows1 <= 0 || cols1 <= 0 || cols2 <= 0 || a == NULL || b == NULL || c == NULL) {
        exit(-1);
    }

    for(int i = 0; i<rows1; i++) {
        for(int j = 0; j<cols2; j++) {
            c[i*cols2 + j] = 0.0f;
            for(int k = 0; k<cols1; k++) {
                c[i*cols2 + j] += a[i*cols1 + k] * b[k*cols2 + j];
            }
        }
    }
}

void matmul_improved(int rows1, int cols1, int cols2, const float *a, const float *b, float *c) {
    if(rows1 <= 0 || cols1 <= 0 || cols2 <= 0 || a == NULL || b == NULL || c == NULL) {
        exit(-1);
    }

    for(int i = 0; i<rows1; i++) {
        for(int k = 0; k<cols1; k++) {
            for(int j = 0; j<cols2; j++) {
                c[i*cols2 + j] += a[i*cols1 + k] * b[k*cols2 + j];
            }
        }
    }
}

void sgemm_OpenBLAS(struct Matrix *A, struct Matrix *B, struct Matrix *C) {
    if(A == NULL || B == NULL || C == NULL || A->data == NULL || B->data == NULL || C->data == NULL || A->cols != B->rows || A->rows != C->rows || B->cols != C->cols) {
        fprintf(stderr, "Invalid matrix pointer provided to sgemm_OpenBLAS\n");
        exit(1);
    }

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                A->rows, B->cols, A->cols,
                1.0f, A->data, A->ld,
                B->data, B->ld,
                0.0f, C->data, C->ld);
}



