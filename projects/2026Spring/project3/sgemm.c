#include "sgemm.h"
#define BLOCK_SIZE 64
#define min(a, b) ((a) < (b) ? (a) : (b))

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

// void matmul_improved(int rows1, int cols1, int cols2, const float *a, const float *b, float *c) {
//     if(rows1 <= 0 || cols1 <= 0 || cols2 <= 0 || a == NULL || b == NULL || c == NULL) {
//         exit(-1);
//     }

//     #pragma omp parallel for
//     for(int i = 0; i<rows1; i++) {
//         for(int k = 0; k<cols1; k++) {
//             float a_ik = a[i*cols1 + k];
//             for(int j = 0; j<cols2; j++) {
//                 c[i*cols2 + j] += a_ik * b[k*cols2 + j];
//             }
//         }
//     }
// }

//blocking 
void matmul_improved(int rows1, int cols1, int cols2, const float *a, const float *b, float *c) {
    if(rows1 <= 0 || cols1 <= 0 || cols2 <= 0 || a == NULL || b == NULL || c == NULL) {
        exit(-1);
    }

    #pragma omp parallel for schedule(static)
    for(int i0 = 0; i0<rows1; i0+=BLOCK_SIZE) {
        for(int k0 = 0; k0<cols1; k0+=BLOCK_SIZE) {
            for(int j0 = 0; j0<cols2; j0+=BLOCK_SIZE) {
                int i_max = min(i0 + BLOCK_SIZE, rows1);
                int k_max = min(k0 + BLOCK_SIZE, cols1);
                int j_max = min(j0 + BLOCK_SIZE, cols2);

                for(int i = i0; i<i_max; i++) {
                    for(int k = k0; k<k_max; k++) {
                        float a_ik = a[i*cols1 + k];
                        for(int j = j0; j<j_max; j++) {
                            c[i*cols2 + j] += a_ik * b[k*cols2 + j];
                        }
                    }
                }
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



