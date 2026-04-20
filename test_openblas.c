#include <cblas.h>
#include <stdio.h>

int main() {
    int M = 2, N = 2, K = 2;
    float A[] = {1.0f, 2.0f, 3.0f, 4.0f};
    float B[] = {5.0f, 6.0f, 7.0f, 8.0f};
    float C[4] = {0.0f};

    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                M, N, K, 1.0f, A, K, B, N, 0.0f, C, N);

    printf("C[0] = %.1f, C[1] = %.1f, C[2] = %.1f, C[3] = %.1f\n",
           C[0], C[1], C[2], C[3]);
    return 0;
}