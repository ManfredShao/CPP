#include "benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *program) {
    printf("Usage: %s [options]\n", program);
    printf("Options:\n");
    printf("  --sizes <list>        Comma-separated list, e.g. 16,128,1024,8192,65536\n");
    printf("  --runs <n>            Number of timing runs for improved/OpenBLAS (default: 3)\n");
    printf("  --plain-max <n>       Max N for plain benchmark (default: 1024)\n");
    printf("  --mem-gib <n>         Memory limit in GiB for one case (default: 8)\n");
    printf("  --seed <n>            RNG seed (default: 42)\n");
    printf("  --csv <path>          Save CSV result to file\n");
    printf("  --no-openblas         Disable OpenBLAS benchmark\n");
    printf("  --help                Show this message\n");
}

static int parse_sizes(const char *text, size_t **sizes_out, size_t *count_out) {
    if (text == NULL || sizes_out == NULL || count_out == NULL) {
        return -1;
    }

    char *buffer = strdup(text);
    if (buffer == NULL) {
        return -1;
    }

    size_t capacity = 8;
    size_t count = 0;
    size_t *sizes = (size_t *)malloc(capacity * sizeof(size_t));
    if (sizes == NULL) {
        free(buffer);
        return -1;
    }

    char *save = NULL;
    char *token = strtok_r(buffer, ",", &save);
    while (token != NULL) {
        char *end = NULL;
        unsigned long long value = strtoull(token, &end, 10);
        if (end == token || *end != '\0' || value == 0) {
            free(sizes);
            free(buffer);
            return -1;
        }
        if (count == capacity) {
            capacity *= 2;
            size_t *new_ptr = (size_t *)realloc(sizes, capacity * sizeof(size_t));
            if (new_ptr == NULL) {
                free(sizes);
                free(buffer);
                return -1;
            }
            sizes = new_ptr;
        }
        sizes[count++] = (size_t)value;
        token = strtok_r(NULL, ",", &save);
    }

    free(buffer);
    if (count == 0) {
        free(sizes);
        return -1;
    }

    *sizes_out = sizes;
    *count_out = count;
    return 0;
}

int main(int argc, char **argv) {
    size_t default_sizes[] = {16, 128, 1024, 8192, 65536};
    size_t *sizes = default_sizes;
    size_t size_count = sizeof(default_sizes) / sizeof(default_sizes[0]);
    int owns_sizes = 0;

    BenchmarkConfig config = {
        .sizes = sizes,
        .size_count = size_count,
        .plain_max_n = 1024,
        .memory_limit_gib = 8,
        .runs = 3,
        .enable_openblas = 1,
        .seed = 42,
        .csv_path = NULL,
    };

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "--no-openblas") == 0) {
            config.enable_openblas = 0;
            continue;
        }
        if (i + 1 >= argc) {
            fprintf(stderr, "Missing value for option: %s\n", argv[i]);
            print_usage(argv[0]);
            if (owns_sizes) {
                free(config.sizes);
            }
            return 1;
        }

        const char *value = argv[++i];
        if (strcmp(argv[i - 1], "--sizes") == 0) {
            size_t *parsed_sizes = NULL;
            size_t parsed_count = 0;
            if (parse_sizes(value, &parsed_sizes, &parsed_count) != 0) {
                fprintf(stderr, "Invalid sizes list: %s\n", value);
                if (owns_sizes) {
                    free(config.sizes);
                }
                return 1;
            }
            if (owns_sizes) {
                free(config.sizes);
            }
            config.sizes = parsed_sizes;
            config.size_count = parsed_count;
            owns_sizes = 1;
        } else if (strcmp(argv[i - 1], "--runs") == 0) {
            config.runs = atoi(value);
            if (config.runs <= 0) {
                fprintf(stderr, "--runs must be > 0\n");
                if (owns_sizes) {
                    free(config.sizes);
                }
                return 1;
            }
        } else if (strcmp(argv[i - 1], "--plain-max") == 0) {
            config.plain_max_n = (size_t)strtoull(value, NULL, 10);
        } else if (strcmp(argv[i - 1], "--mem-gib") == 0) {
            config.memory_limit_gib = (size_t)strtoull(value, NULL, 10);
        } else if (strcmp(argv[i - 1], "--seed") == 0) {
            config.seed = (uint32_t)strtoul(value, NULL, 10);
        } else if (strcmp(argv[i - 1], "--csv") == 0) {
            config.csv_path = value;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i - 1]);
            print_usage(argv[0]);
            if (owns_sizes) {
                free(config.sizes);
            }
            return 1;
        }
    }

    int rc = run_benchmarks(&config);
    if (owns_sizes) {
        free(config.sizes);
    }
    return rc == 0 ? 0 : 1;
}
