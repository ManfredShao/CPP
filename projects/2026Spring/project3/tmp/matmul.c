#include "matmul.h"

#include <stddef.h>
#include <stdlib.h>

#if defined(USE_OPENBLAS)
#if defined(__APPLE__)
#include <Accelerate/Accelerate.h>
#else
#include <cblas.h>
#endif
#endif

#if defined(__AVX2__)
#include <immintrin.h>
#endif

#if defined(__ARM_NEON)
#include <arm_neon.h>
#endif

static int validate_inputs(const Matrix *a, const Matrix *b, Matrix *c) {
    if (a == NULL || b == NULL || c == NULL || a->data == NULL || b->data == NULL || c->data == NULL) {
        return MATMUL_ERR_INVALID_ARG;
    }
    if (a->cols != b->rows) {
        return MATMUL_ERR_DIM_MISMATCH;
    }
    if (c->rows != a->rows || c->cols != b->cols) {
        return MATMUL_ERR_DIM_MISMATCH;
    }
    return MATMUL_OK;
}

int matmul_plain(const Matrix *a, const Matrix *b, Matrix *c) {
    int status = validate_inputs(a, b, c);
    if (status != MATMUL_OK) {
        return status;
    }

    const size_t m = a->rows;
    const size_t k = a->cols;
    const size_t n = b->cols;

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            c->data[i * n + j] = 0.0f;
        }
    }

    for (size_t i = 0; i < m; ++i) {
        for (size_t p = 0; p < k; ++p) {
            const float a_ip = a->data[i * k + p];
            const float *b_row = &b->data[p * n];
            float *c_row = &c->data[i * n];
            for (size_t j = 0; j < n; ++j) {
                c_row[j] += a_ip * b_row[j];
            }
        }
    }
    return MATMUL_OK;
}

static float dot_product_simd(const float *lhs, const float *rhs, size_t count) {
    size_t index = 0;

#if defined(__AVX2__)
    __m256 accum8 = _mm256_setzero_ps();
    for (; index + 8 <= count; index += 8) {
        __m256 va = _mm256_loadu_ps(lhs + index);
        __m256 vb = _mm256_loadu_ps(rhs + index);
        accum8 = _mm256_fmadd_ps(va, vb, accum8);
    }
    __m128 lo = _mm256_castps256_ps128(accum8);
    __m128 hi = _mm256_extractf128_ps(accum8, 1);
    __m128 sum4 = _mm_add_ps(lo, hi);
    sum4 = _mm_hadd_ps(sum4, sum4);
    sum4 = _mm_hadd_ps(sum4, sum4);
    float sum = _mm_cvtss_f32(sum4);
#elif defined(__ARM_NEON)
    float32x4_t accum4 = vdupq_n_f32(0.0f);
    for (; index + 4 <= count; index += 4) {
        float32x4_t va = vld1q_f32(lhs + index);
        float32x4_t vb = vld1q_f32(rhs + index);
        accum4 = vfmaq_f32(accum4, va, vb);
    }
#if defined(__aarch64__)
    float sum = vaddvq_f32(accum4);
#else
    float32x2_t pair = vadd_f32(vget_low_f32(accum4), vget_high_f32(accum4));
    pair = vpadd_f32(pair, pair);
    float sum = vget_lane_f32(pair, 0);
#endif
#else
    float sum = 0.0f;
#endif

#pragma omp simd reduction(+ : sum)
    for (; index < count; ++index) {
        sum += lhs[index] * rhs[index];
    }

    return sum;
}

int matmul_improved(const Matrix *a, const Matrix *b, Matrix *c) {
    int status = validate_inputs(a, b, c);
    if (status != MATMUL_OK) {
        return status;
    }

    const size_t m = a->rows;
    const size_t k = a->cols;
    const size_t n = b->cols;

    const size_t bt_elements = n * k;
    if (k != 0 && bt_elements / k != n) {
        return MATMUL_ERR_NO_MEMORY;
    }

    float *bt = (float *)malloc(bt_elements * sizeof(float));
    if (bt == NULL) {
        return MATMUL_ERR_NO_MEMORY;
    }

    for (size_t row = 0; row < k; ++row) {
        for (size_t col = 0; col < n; ++col) {
            bt[col * k + row] = b->data[row * n + col];
        }
    }

    const size_t block_m = 64;
    const size_t block_n = 64;
    const size_t block_k = 256;

#pragma omp parallel for collapse(2) schedule(static)
    for (size_t ii = 0; ii < m; ii += block_m) {
        for (size_t jj = 0; jj < n; jj += block_n) {
            size_t i_end = ii + block_m < m ? ii + block_m : m;
            size_t j_end = jj + block_n < n ? jj + block_n : n;

            for (size_t i = ii; i < i_end; ++i) {
                const float *a_row = &a->data[i * k];
                float *c_row = &c->data[i * n];
                for (size_t j = jj; j < j_end; ++j) {
                    float sum = 0.0f;
                    const float *bt_row = &bt[j * k];
                    for (size_t kk = 0; kk < k; kk += block_k) {
                        size_t k_end = kk + block_k < k ? kk + block_k : k;
                        sum += dot_product_simd(a_row + kk, bt_row + kk, k_end - kk);
                    }
                    c_row[j] = sum;
                }
            }
        }
    }

    free(bt);
    return MATMUL_OK;
}

int matmul_openblas(const Matrix *a, const Matrix *b, Matrix *c) {
    int status = validate_inputs(a, b, c);
    if (status != MATMUL_OK) {
        return status;
    }

#if defined(USE_OPENBLAS)
    cblas_sgemm(CblasRowMajor,
                CblasNoTrans,
                CblasNoTrans,
                (int)a->rows,
                (int)b->cols,
                (int)a->cols,
                1.0f,
                a->data,
                (int)a->cols,
                b->data,
                (int)b->cols,
                0.0f,
                c->data,
                (int)c->cols);
    return MATMUL_OK;
#else
    (void)a;
    (void)b;
    (void)c;
    return MATMUL_ERR_UNAVAILABLE;
#endif
}
