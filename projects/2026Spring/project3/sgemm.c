#include "sgemm.h"
#include <malloc.h>   // for _aligned_malloc
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>  // AVX2
#endif

// #include <valgrind/callgrind.h>
#include <omp.h>
#include <string.h>
#include <stdlib.h>
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
    
    // CALLGRIND_START_INSTRUMENTATION;
#pragma omp parallel 
{
    float* B_pack = (float *)_aligned_malloc(block_size * block_size * sizeof(float), 32);
    #pragma omp for schedule(static)
    for(int i0 = 0; i0<rows1; i0+=block_size) {
        for(int k0 = 0; k0<cols1; k0+=block_size) {
            for(int j0 = 0; j0<cols2; j0+=block_size) {
                int i_max = min(i0 + block_size, rows1);
                int k_max = min(k0 + block_size, cols1);
                int j_max = min(j0 + block_size, cols2);
                int j_len = j_max - j0;

                for(int k = k0; k<k_max; k++) {
                    const float* src = b + k * cols2 + j0;
                    float* dst = B_pack + (k-k0) * j_len;
                    memcpy(dst, src, j_len * sizeof(float));
                }

                int i = i0;
                for(; i <= i_max - 4; i+=4) {
                    const float *ap0 = a + (i + 0) * cols1;
                    const float *ap1 = a + (i + 1) * cols1;
                    const float *ap2 = a + (i + 2) * cols1;
                    const float *ap3 = a + (i + 3) * cols1;
                    float *cp0 = c + (i + 0) * cols2;
                    float *cp1 = c + (i + 1) * cols2;
                    float *cp2 = c + (i + 2) * cols2;
                    float *cp3 = c + (i + 3) * cols2;

                    int j=0;
                    for( ; j<= j_len - 16;j+=16) {
                        __m256 c00 = _mm256_loadu_ps(cp0 + j0 + j);
                        __m256 c01 = _mm256_loadu_ps(cp0 + j0 + j + 8);
                        __m256 c10 = _mm256_loadu_ps(cp1 + j0 + j);
                        __m256 c11 = _mm256_loadu_ps(cp1 + j0 + j + 8);
                        __m256 c20 = _mm256_loadu_ps(cp2 + j0 + j);
                        __m256 c21 = _mm256_loadu_ps(cp2 + j0 + j + 8);
                        __m256 c30 = _mm256_loadu_ps(cp3 + j0 + j);
                        __m256 c31 = _mm256_loadu_ps(cp3 + j0 + j + 8);

                        for(int k = k0; k < k_max; k++) {
                            // 加载 B 的 16 个元素
                            const float *bp = B_pack + (k - k0) * j_len + j;
                            __m256 b0 = _mm256_loadu_ps(bp);
                            __m256 b1 = _mm256_loadu_ps(bp + 8);

                            // 第 0 行 FMA
                            __m256 a0 = _mm256_set1_ps(ap0[k]);
                            c00 = _mm256_fmadd_ps(a0, b0, c00);
                            c01 = _mm256_fmadd_ps(a0, b1, c01);

                            // 第 1 行 FMA
                            __m256 a1 = _mm256_set1_ps(ap1[k]);
                            c10 = _mm256_fmadd_ps(a1, b0, c10);
                            c11 = _mm256_fmadd_ps(a1, b1, c11);

                            // 第 2 行 FMA
                            __m256 a2 = _mm256_set1_ps(ap2[k]);
                            c20 = _mm256_fmadd_ps(a2, b0, c20);
                            c21 = _mm256_fmadd_ps(a2, b1, c21);

                            // 第 3 行 FMA
                            __m256 a3 = _mm256_set1_ps(ap3[k]);
                            c30 = _mm256_fmadd_ps(a3, b0, c30);
                            c31 = _mm256_fmadd_ps(a3, b1, c31);
                        }

                        // 3. 统一写回内存
                        _mm256_storeu_ps(cp0 + j0 + j, c00);
                        _mm256_storeu_ps(cp0 + j0 + j + 8, c01);
                        _mm256_storeu_ps(cp1 + j0 + j, c10);
                        _mm256_storeu_ps(cp1 + j0 + j + 8, c11);
                        _mm256_storeu_ps(cp2 + j0 + j, c20);
                        _mm256_storeu_ps(cp2 + j0 + j + 8, c21);
                        _mm256_storeu_ps(cp3 + j0 + j, c30);
                        _mm256_storeu_ps(cp3 + j0 + j + 8, c31);
                    }

                    for (; j < j_len; j++) {
                        float sum0 = cp0[j0 + j];
                        float sum1 = cp1[j0 + j];
                        float sum2 = cp2[j0 + j];
                        float sum3 = cp3[j0 + j];

                        for(int k = k0; k < k_max; k++) {
                            float b_val = B_pack[(k - k0) * j_len + j];
                            sum0 += ap0[k] * b_val;
                            sum1 += ap1[k] * b_val;
                            sum2 += ap2[k] * b_val;
                            sum3 += ap3[k] * b_val;
                        }

                        cp0[j0 + j] = sum0;
                        cp1[j0 + j] = sum1;
                        cp2[j0 + j] = sum2;
                        cp3[j0 + j] = sum3;
                    }
                }

                for(; i < i_max; i++) {
                    const float *ap = a + i * cols1;
                    float *cp = c + i * cols2;
                    
                    int j_res = 0;
                    // --- 2.1 针对残余行的 1x8 AVX2 处理 ---
                    for (; j_res <= j_len - 8; j_res += 8) {
                        // 循环外读取一次
                        __m256 vc = _mm256_loadu_ps(cp + j0 + j_res);
                        
                        // K 循环纯寄存器计算
                        for(int k = k0; k < k_max; k++) {
                            __m256 va = _mm256_set1_ps(ap[k]);
                            const float *bp = B_pack + (k - k0) * j_len + j_res;
                            // 注意：这里由于是残余部分，为了安全用 loadu_ps，如果 B_pack 严格对齐可换 load_ps
                            vc = _mm256_fmadd_ps(va, _mm256_loadu_ps(bp), vc); 
                        }
                        
                        // 循环外写回一次
                        _mm256_storeu_ps(cp + j0 + j_res, vc);
                    }
                    
                    // --- 2.2 残余行的列残余部分 (绝对边缘，纯标量) ---
                    for(; j_res < j_len; j_res++) {
                        float sum = cp[j0 + j_res];
                        for(int k = k0; k < k_max; k++) {
                            sum += ap[k] * B_pack[(k - k0) * j_len + j_res];
                        }
                        cp[j0 + j_res] = sum;
                    }
                }
            }
        }
    }
    _aligned_free(B_pack);
}
// CALLGRIND_STOP_INSTRUMENTATION;
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



