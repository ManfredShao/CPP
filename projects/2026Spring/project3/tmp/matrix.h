#ifndef MATRIX_H
#define MATRIX_H

#include <stddef.h>
#include <stdint.h>

typedef struct Matrix {
    size_t rows;
    size_t cols;
    float *data;
} Matrix;

int matrix_init(Matrix *matrix, size_t rows, size_t cols);
void matrix_free(Matrix *matrix);
void matrix_fill_zero(Matrix *matrix);
void matrix_fill_random(Matrix *matrix, uint32_t *state);
float matrix_max_abs_diff(const Matrix *lhs, const Matrix *rhs);
float matrix_max_rel_diff(const Matrix *lhs, const Matrix *rhs);
size_t matrix_total_bytes(size_t rows, size_t cols);

#endif
