#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>
#include <stdint.h>

typedef struct BenchmarkConfig {
    size_t *sizes;
    size_t size_count;
    size_t plain_max_n;
    size_t memory_limit_gib;
    int runs;
    int enable_openblas;
    uint32_t seed;
    const char *csv_path;
} BenchmarkConfig;

int run_benchmarks(const BenchmarkConfig *config);

#endif
