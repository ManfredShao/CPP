#ifndef MATMUL_H
#define MATMUL_H

#include "matrix.h"

enum {
    MATMUL_OK = 0,
    MATMUL_ERR_INVALID_ARG = -1,
    MATMUL_ERR_DIM_MISMATCH = -2,
    MATMUL_ERR_NO_MEMORY = -3,
    MATMUL_ERR_UNAVAILABLE = -4
};

int matmul_plain(const Matrix *a, const Matrix *b, Matrix *c);
int matmul_improved(const Matrix *a, const Matrix *b, Matrix *c);
int matmul_openblas(const Matrix *a, const Matrix *b, Matrix *c);

#endif
