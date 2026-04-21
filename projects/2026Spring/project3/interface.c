#include "sgemm.h"
#include <math.h>

struct Matrix* generate_matrix(size_t rows, size_t cols) {
    struct Matrix *mat = (struct Matrix*)malloc(sizeof(struct Matrix));
    if(mat == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix structure\n");
        return NULL;
    }

    mat->rows = rows;
    mat->cols = cols;
    mat->ld = cols; // leading dimension for BLAS is typically the number of columns
    mat->data = (float*)malloc(rows * cols * sizeof(float));
    if(mat->data == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix data\n");
        free(mat);
        return NULL;
    }

    for(size_t i = 0; i < rows * cols; i++) {
        mat->data[i] = (float)(rand() / RAND_MAX); // Initialize with random float values between 0 and 1
    }

    return mat;
}

void freeMatrix(struct Matrix *mat) {
    if(mat != NULL) {
        free(mat->data);
        free(mat);
    }
}

void check_result(struct Matrix *C1, struct Matrix *C2, struct Matrix *C3, size_t rows, size_t cols) {
    if(C1 == NULL || C2 == NULL || C3 == NULL || C1->data == NULL || C2->data == NULL || C3->data == NULL) {
        fprintf(stderr, "Invalid matrix pointer provided to check_result\n");
        exit(1);
    }

    for(size_t i = 0; i < rows * cols; i++) {
        if(fabs(C1->data[i] - C2->data[i]) > 1e-5 || fabs(C1->data[i] - C3->data[i]) > 1e-5) {
            fprintf(stderr, "Results do not match at index %zu: C1=%f, C2=%f, C3=%f\n", i, C1->data[i], C2->data[i], C3->data[i]);
            exit(1);
        }
    }
    printf("Results match for all elements.\n");
}

void sgemm(struct Matrix *A, struct Matrix *B, struct Matrix *C, int type) {
    if(A == NULL || B == NULL || C == NULL || A->data == NULL || B->data == NULL || C->data == NULL || A->cols != B->rows || A->rows != C->rows || B->cols != C->cols) {
        fprintf(stderr, "Invalid matrix pointer provided to sgemm\n");
        exit(1);
    }

    double start_time = omp_get_wtime();
    if(type == 1) {
        matmul_plain(A->rows, A->cols, B->cols, A->data, B->data, C->data);
    } else if(type == 2) {
        matmul_improved(A->rows, A->cols, B->cols, A->data, B->data, C->data);
    } else if(type == 3) {
        sgemm_OpenBLAS(A, B, C);
    } else {
        fprintf(stderr, "Invalid matrix multiplication type specified\n");
        exit(1);
    }
    double end_time = omp_get_wtime();
    printf("Time: %f seconds\n", end_time - start_time);
}