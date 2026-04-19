#include "matrix.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

size_t matrix_total_bytes(size_t rows, size_t cols) {
    if (rows == 0 || cols == 0) {
        return 0;
    }
    if (rows > SIZE_MAX / cols) {
        return 0;
    }
    size_t elements = rows * cols;
    if (elements > SIZE_MAX / sizeof(float)) {
        return 0;
    }
    return elements * sizeof(float);
}

int matrix_init(Matrix *matrix, size_t rows, size_t cols) {
    if (matrix == NULL || rows == 0 || cols == 0) {
        return -1;
    }
    size_t bytes = matrix_total_bytes(rows, cols);
    if (bytes == 0) {
        return -1;
    }

    void *memory = NULL;
#if defined(_ISOC11_SOURCE)
    memory = aligned_alloc(64, ((bytes + 63) / 64) * 64);
    if (memory == NULL) {
        return -2;
    }
#else
    if (posix_memalign(&memory, 64, bytes) != 0) {
        return -2;
    }
#endif

    matrix->rows = rows;
    matrix->cols = cols;
    matrix->data = (float *)memory;
    memset(matrix->data, 0, bytes);
    return 0;
}

void matrix_free(Matrix *matrix) {
    if (matrix == NULL) {
        return;
    }
    free(matrix->data);
    matrix->data = NULL;
    matrix->rows = 0;
    matrix->cols = 0;
}

void matrix_fill_zero(Matrix *matrix) {
    if (matrix == NULL || matrix->data == NULL) {
        return;
    }
    size_t bytes = matrix_total_bytes(matrix->rows, matrix->cols);
    if (bytes > 0) {
        memset(matrix->data, 0, bytes);
    }
}

void matrix_fill_random(Matrix *matrix, uint32_t *state) {
    if (matrix == NULL || matrix->data == NULL || state == NULL) {
        return;
    }
    size_t count = matrix->rows * matrix->cols;
    for (size_t i = 0; i < count; ++i) {
        uint32_t value = xorshift32(state);
        float normalized = (float)(value & 0x00FFFFFFu) / 16777216.0f;
        matrix->data[i] = normalized * 2.0f - 1.0f;
    }
}

float matrix_max_abs_diff(const Matrix *lhs, const Matrix *rhs) {
    if (lhs == NULL || rhs == NULL || lhs->data == NULL || rhs->data == NULL) {
        return INFINITY;
    }
    if (lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
        return INFINITY;
    }
    size_t count = lhs->rows * lhs->cols;
    float max_diff = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float diff = fabsf(lhs->data[i] - rhs->data[i]);
        if (diff > max_diff) {
            max_diff = diff;
        }
    }
    return max_diff;
}

float matrix_max_rel_diff(const Matrix *lhs, const Matrix *rhs) {
    if (lhs == NULL || rhs == NULL || lhs->data == NULL || rhs->data == NULL) {
        return INFINITY;
    }
    if (lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
        return INFINITY;
    }
    size_t count = lhs->rows * lhs->cols;
    float max_rel = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        float a = lhs->data[i];
        float b = rhs->data[i];
        float denom = fmaxf(fabsf(a), 1e-8f);
        float rel = fabsf(a - b) / denom;
        if (rel > max_rel) {
            max_rel = rel;
        }
    }
    return max_rel;
}
