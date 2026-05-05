#include "sgemm.h"
#include <math.h>

struct Matrix* generate_matrix(size_t rows, size_t cols) {
    struct Matrix *mat = (struct Matrix*)malloc(sizeof(struct Matrix));
    if(mat == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix structure\n");
        exit(1);
    }

    mat->rows = rows;
    mat->cols = cols;
    mat->ld = cols; // leading dimension for BLAS is typically the number of columns
    mat->data = (float*)malloc(rows * cols * sizeof(float));
    if(mat->data == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix data\n");
        free(mat);
        exit(1);
    }

    for(size_t i = 0; i < rows * cols; i++) {
        mat->data[i] = (float)rand() / (float)RAND_MAX; // Initialize with random float values between 0 and 1
    }

    return mat;
}

struct Matrix* create_matrix(size_t rows, size_t cols) {
    struct Matrix *mat = (struct Matrix*)malloc(sizeof(struct Matrix));
    if(mat == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix structure\n");
        exit(1);
    }

    mat->rows = rows;
    mat->cols = cols;
    mat->ld = cols; 
    mat->data = (float*)malloc(rows * cols * sizeof(float));
    if(mat->data == NULL) {
        fprintf(stderr, "Memory allocation failed for matrix data\n");
        free(mat);
        exit(1);
    }

    memset(mat->data, 0, rows * cols * sizeof(float));

    return mat;
}

void freeMatrix(struct Matrix *mat) {
    if(mat != NULL) {
        free(mat->data);
        free(mat);
    }
}

void check_result(__attribute__((unused)) struct Matrix *C1, struct Matrix *C2, struct Matrix *C3, size_t rows, size_t cols) {
    if(C2 == NULL || C3 == NULL || C2->data == NULL || C3->data == NULL) {
        fprintf(stderr, "Invalid matrix pointer provided to check_result\n");
        exit(1);
    }

    for(size_t i = 0; i < rows * cols; i++) {
        float v2 = C2->data[i];
        float v3 = C3->data[i];
        float diff = fabs(v2 - v3);
        float max_val = fabs(v2) > fabs(v3) ? fabs(v2) : fabs(v3);
        
        // relative error check with a small epsilon to avoid division by zero, and absolute error check.
        if(diff / (max_val + 1e-8) > 1e-3 && diff > 1e-4) {
            fprintf(stderr, "Results do not match at index %zu: C2=%f, C3=%f\n", i, v2, v3);
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
    printf("type: %d Time: %f seconds\n", type, end_time - start_time);
}