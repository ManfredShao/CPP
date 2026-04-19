#include "benchmark.h"

#include "matmul.h"
#include "matrix.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_OPENMP)
#include <omp.h>
#endif

static double now_seconds(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static size_t total_required_bytes(size_t n) {
    size_t one = matrix_total_bytes(n, n);
    if (one == 0) {
        return 0;
    }
    if (one > SIZE_MAX / 4) {
        return 0;
    }
    return one * 4;
}

static int parse_memory_limit_bytes(size_t gib, size_t *bytes_out) {
    if (bytes_out == NULL) {
        return -1;
    }
    if (gib > SIZE_MAX / (1024ull * 1024ull * 1024ull)) {
        return -1;
    }
    *bytes_out = gib * 1024ull * 1024ull * 1024ull;
    return 0;
}

static double benchmark_once(const Matrix *a, const Matrix *b, Matrix *c, int (*kernel)(const Matrix *, const Matrix *, Matrix *), int runs, int *status_out) {
    if (status_out == NULL || runs <= 0) {
        return -1.0;
    }

    *status_out = kernel(a, b, c);
    if (*status_out != MATMUL_OK) {
        return -1.0;
    }

    double best = 1e100;
    for (int i = 0; i < runs; ++i) {
        double start = now_seconds();
        *status_out = kernel(a, b, c);
        double elapsed = now_seconds() - start;
        if (elapsed <= 0.0) {
            elapsed = 1e-9;
        }
        if (*status_out != MATMUL_OK) {
            return -1.0;
        }
        if (elapsed < best) {
            best = elapsed;
        }
    }
    return best;
}

int run_benchmarks(const BenchmarkConfig *config) {
    if (config == NULL || config->sizes == NULL || config->size_count == 0 || config->runs <= 0) {
        return -1;
    }

    size_t memory_limit_bytes = 0;
    if (parse_memory_limit_bytes(config->memory_limit_gib, &memory_limit_bytes) != 0) {
        return -1;
    }

    FILE *csv = NULL;
    if (config->csv_path != NULL) {
        csv = fopen(config->csv_path, "w");
        if (csv == NULL) {
            fprintf(stderr, "Failed to open CSV path '%s': %s\n", config->csv_path, strerror(errno));
            return -1;
        }
        fprintf(csv, "n,plain_s,improved_s,openblas_s,plain_gflops,improved_gflops,openblas_gflops,speedup_over_plain,speedup_vs_openblas,max_abs_diff_plain,max_rel_diff_plain,max_abs_diff_openblas,max_rel_diff_openblas\n");
    }

#if defined(_OPENMP)
    int threads = omp_get_max_threads();
#else
    int threads = 1;
#endif
    printf("Benchmark settings: runs=%d, plain_max_n=%zu, memory_limit=%zu GiB, openblas=%s, threads=%d\n",
           config->runs,
           config->plain_max_n,
           config->memory_limit_gib,
           config->enable_openblas ? "on" : "off",
           threads);
    printf("%-10s %-12s %-12s %-12s %-12s %-12s %-12s\n",
           "N",
           "plain(s)",
           "improved(s)",
           "openblas(s)",
           "imp GFLOPS",
           "x/plain",
           "x/openblas");

    uint32_t seed = config->seed;

    for (size_t idx = 0; idx < config->size_count; ++idx) {
        size_t n = config->sizes[idx];
        size_t bytes_needed = total_required_bytes(n);
        if (bytes_needed == 0) {
            fprintf(stderr, "[N=%zu] size overflow, skip.\n", n);
            continue;
        }
        if (bytes_needed > memory_limit_bytes) {
            printf("%-10zu %-12s %-12s %-12s %-12s %-12s %-12s\n", n, "SKIP", "SKIP", "SKIP", "-", "-", "-");
            continue;
        }

        Matrix a = {0}, b = {0}, c_plain = {0}, c_improved = {0}, c_openblas = {0};
        if (matrix_init(&a, n, n) != 0 || matrix_init(&b, n, n) != 0 ||
            matrix_init(&c_plain, n, n) != 0 || matrix_init(&c_improved, n, n) != 0 || matrix_init(&c_openblas, n, n) != 0) {
            fprintf(stderr, "[N=%zu] matrix allocation failed, skip.\n", n);
            matrix_free(&a);
            matrix_free(&b);
            matrix_free(&c_plain);
            matrix_free(&c_improved);
            matrix_free(&c_openblas);
            continue;
        }

        matrix_fill_random(&a, &seed);
        matrix_fill_random(&b, &seed);

        int status = MATMUL_OK;
        double improved_s = benchmark_once(&a, &b, &c_improved, matmul_improved, config->runs, &status);
        if (status != MATMUL_OK) {
            fprintf(stderr, "[N=%zu] matmul_improved failed (%d).\n", n, status);
            matrix_free(&a);
            matrix_free(&b);
            matrix_free(&c_plain);
            matrix_free(&c_improved);
            matrix_free(&c_openblas);
            continue;
        }

        double plain_s = -1.0;
        float abs_plain = -1.0f;
        float rel_plain = -1.0f;
        if (n <= config->plain_max_n) {
            plain_s = benchmark_once(&a, &b, &c_plain, matmul_plain, 1, &status);
            if (status == MATMUL_OK) {
                abs_plain = matrix_max_abs_diff(&c_improved, &c_plain);
                rel_plain = matrix_max_rel_diff(&c_improved, &c_plain);
            }
        }

        double openblas_s = -1.0;
        float abs_openblas = -1.0f;
        float rel_openblas = -1.0f;
        if (config->enable_openblas) {
            openblas_s = benchmark_once(&a, &b, &c_openblas, matmul_openblas, config->runs, &status);
            if (status == MATMUL_OK) {
                abs_openblas = matrix_max_abs_diff(&c_improved, &c_openblas);
                rel_openblas = matrix_max_rel_diff(&c_improved, &c_openblas);
            } else {
                openblas_s = -1.0;
            }
        }

        double flops = 2.0 * (double)n * (double)n * (double)n;
        double improved_gflops = flops / improved_s / 1e9;
        double plain_gflops = plain_s > 0.0 ? flops / plain_s / 1e9 : -1.0;
        double openblas_gflops = openblas_s > 0.0 ? flops / openblas_s / 1e9 : -1.0;
        double speedup_plain = plain_s > 0.0 ? plain_s / improved_s : -1.0;
        double speedup_openblas = openblas_s > 0.0 ? openblas_s / improved_s : -1.0;

        char openblas_text[32];
        char speedup_plain_text[32];
        char speedup_openblas_text[32];
        if (openblas_s > 0.0) {
            snprintf(openblas_text, sizeof(openblas_text), "%.6f", openblas_s);
        } else {
            snprintf(openblas_text, sizeof(openblas_text), "N/A");
        }
        if (speedup_plain > 0.0) {
            snprintf(speedup_plain_text, sizeof(speedup_plain_text), "%.2f", speedup_plain);
        } else {
            snprintf(speedup_plain_text, sizeof(speedup_plain_text), "N/A");
        }
        if (speedup_openblas > 0.0) {
            snprintf(speedup_openblas_text, sizeof(speedup_openblas_text), "%.2f", speedup_openblas);
        } else {
            snprintf(speedup_openblas_text, sizeof(speedup_openblas_text), "N/A");
        }

        printf("%-10zu %-12.6f %-12.6f %-12s %-12.2f %-12s %-12s\n",
               n,
               plain_s > 0 ? plain_s : 0.0,
               improved_s,
               openblas_text,
               improved_gflops,
               speedup_plain_text,
               speedup_openblas_text);

        if (plain_s > 0.0) {
            printf("  plain: %.6fs, %.2f GFLOPS, speedup %.2fx, diff(abs=%.3e rel=%.3e)\n",
                   plain_s,
                   plain_gflops,
                   speedup_plain,
                   abs_plain,
                   rel_plain);
        } else {
            printf("  plain: skipped for N=%zu (plain_max_n=%zu)\n", n, config->plain_max_n);
        }
        if (openblas_s > 0.0) {
            printf("  openblas: %.6fs, %.2f GFLOPS, improved/openblas %.2fx, diff(abs=%.3e rel=%.3e)\n",
                   openblas_s,
                   openblas_gflops,
                   speedup_openblas,
                   abs_openblas,
                   rel_openblas);
        } else if (config->enable_openblas) {
            printf("  openblas: unavailable or not linked (rebuild with -DUSE_OPENBLAS=ON)\n");
        }

        if (csv != NULL) {
            fprintf(csv,
                    "%zu,%.9f,%.9f,%.9f,%.6f,%.6f,%.6f,%.6f,%.6f,%.9e,%.9e,%.9e,%.9e\n",
                    n,
                    plain_s,
                    improved_s,
                    openblas_s,
                    plain_gflops,
                    improved_gflops,
                    openblas_gflops,
                    speedup_plain,
                    speedup_openblas,
                    abs_plain,
                    rel_plain,
                    abs_openblas,
                    rel_openblas);
        }

        matrix_free(&a);
        matrix_free(&b);
        matrix_free(&c_plain);
        matrix_free(&c_improved);
        matrix_free(&c_openblas);
    }

    if (csv != NULL) {
        fclose(csv);
    }
    return 0;
}
