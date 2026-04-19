#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define REPEAT 10

double dot_product_double(const double *a, const double *b, int n);
double dot_product_float(const float *a, const float *b, int n);
long long dot_product_int(const int *a, const int *b, int n);
long long dot_product_short(const short *a, const short *b, int n);
long long dot_product_signed_char(const signed char *a, const signed char *b, int n);

void test_double_performance(size_t n, FILE *fp);
void test_float_performance(size_t n, FILE *fp);
void test_int_performance(size_t n, FILE *fp);
void test_short_performance(size_t n, FILE *fp);
void test_signed_char_performance(size_t n, FILE *fp);

double get_time_sec();

int main()
{
    // set seed for random number generator
    srand((unsigned int)(time(NULL)));
    FILE *fp = fopen("c_results.csv", "w");  
    if (fp == NULL) {
        fprintf(stderr, "Error opening file for writing\n");
        return EXIT_FAILURE;
    }
    fprintf(fp, "lang,type,length,avg_time_s\n");
    fflush(fp);

    printf("%-4s | %-12s | %10s | %12s | %s\n", "Lang", "Type", "Length", "Avg Time(s)", "Result");
    printf("-----+--------------+------------+--------------+----------------\n");

    size_t lengths[] = {1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8};
    int num_lengths = sizeof(lengths) / sizeof(lengths[0]);

    for (int i = 0; i < num_lengths; i++)
    {
        test_double_performance(lengths[i], fp);
        test_float_performance(lengths[i], fp);
        test_int_performance(lengths[i], fp);
        test_short_performance(lengths[i], fp);
        test_signed_char_performance(lengths[i], fp);
    }

    fclose(fp);
    return 0;
}

double dot_product_double(const double *a, const double *b, int n)
{
    double result = 0.0;
    for (int i = 0; i < n; i++)
    {
        result += a[i] * b[i];
    }
    return result;
}

double dot_product_float(const float *a, const float *b, int n)
{
    double result = 0.0;
    for (int i = 0; i < n; i++)
    {
        result += a[i] * b[i];
    }
    return result;
}

long long dot_product_int(const int *a, const int *b, int n)
{
    long long result = 0;
    for (int i = 0; i < n; i++)
    {
        result += (long long) a[i] * b[i];
    }
    return result;
}

long long dot_product_short(const short *a, const short *b, int n)
{
    long long result = 0;
    for (int i = 0; i < n; i++)
    {
        result += (long long) a[i] * b[i];
    }
    return result;
}

long long dot_product_signed_char(const signed char *a, const signed char *b, int n)
{
    long long result = 0;
    for (int i = 0; i < n; i++)
    {
        result += (long long) a[i] * b[i];
    }
    return result;
}

void test_double_performance(size_t n, FILE *fp)
{
    double *a = (double *)malloc(n * sizeof(double));
    double *b = (double *)malloc(n * sizeof(double));
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed for double arrays of size %zu\n", n);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++) {
        a[i] = 2.0 *  (double)rand() / RAND_MAX - 1.0; // random double in range [-1.0, 1.0]
        b[i] = 2.0 * (double)rand() / RAND_MAX - 1.0; 
    }

    double total = 0.0;
    double last_result = 0.0;
    for (int r = 0; r < REPEAT; r++) {
        double start_time = get_time_sec();
        last_result = dot_product_double(a, b, (int)n);
        double end_time = get_time_sec();
        double elapsed_time = end_time - start_time;
        total += elapsed_time;
    }

    double avg_time = total / REPEAT;
    printf("%-4s | %-12s | %10zu | %12.6f | % .6f\n", "C", "double", n, avg_time, last_result);

    if (fp != NULL) {
        fprintf(fp, "C,double,%zu,%.6f\n", n, avg_time);
        fflush(fp);
    }

    free(a);
    free(b);
    a = NULL;
    b = NULL;
}

void test_float_performance(size_t n, FILE *fp)
{
    float *a = (float *)malloc(n * sizeof(float));
    float *b = (float *)malloc(n * sizeof(float));
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed for float arrays of size %zu\n", n);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++) {
        a[i] = 2.0f * (float)rand() / RAND_MAX - 1.0f; // random float in range [-1.0, 1.0]
        b[i] = 2.0f * (float)rand() / RAND_MAX - 1.0f; 
    }

    double total = 0.0;
    double last_result = 0.0;
    for (int r = 0; r < REPEAT; r++) {
        double start_time = get_time_sec();
        last_result = dot_product_float(a, b, (int)n);
        double end_time = get_time_sec();
        double elapsed_time = end_time - start_time;
        total += elapsed_time;
    }

    double avg_time = total / REPEAT;
    printf("%-4s | %-12s | %10zu | %12.6f | % .6f\n", "C", "float", n, avg_time, last_result);

    if (fp != NULL) {
        fprintf(fp, "C,float,%zu,%.6f\n", n, avg_time);
        fflush(fp);
    }

    free(a);
    free(b);
    a = NULL;
    b = NULL;   
}

void test_int_performance(size_t n, FILE *fp)
{
    int *a = (int *)malloc(n * sizeof(int));
    int *b = (int *)malloc(n * sizeof(int));
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed for int arrays of size %zu\n", n);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++) {
        a[i] = rand() % 2001 - 1000; // random int in range [-1000, 1000]
        b[i] = rand() % 2001 - 1000; 
    }
    double total = 0.0;
    long long last_result = 0;
    for (int r = 0; r < REPEAT; r++) {
        double start_time = get_time_sec();
        last_result = dot_product_int(a, b, (int)n);
        double end_time = get_time_sec();
        double elapsed_time = end_time - start_time;
        total += elapsed_time;
    }

    double avg_time = total / REPEAT;
    printf("%-4s | %-12s | %10zu | %12.6f | %lld\n", "C", "int", n, avg_time, last_result);

    if (fp != NULL) {
        fprintf(fp, "C,int,%zu,%.6f\n", n, avg_time);
        fflush(fp);
    }

    free(a);
    free(b);
    a = NULL;
    b = NULL;   
}

void test_short_performance(size_t n, FILE *fp)
{
    short *a = (short *)malloc(n * sizeof(short));
    short *b = (short *)malloc(n * sizeof(short));
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed for short arrays of size %zu\n", n);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++) {
        a[i] = rand() % 201 - 100; // random short in range [-100, 100]
        b[i] = rand() % 201 - 100; 
    }
    double total = 0.0;
    long long last_result = 0;
    for (int r = 0; r < REPEAT; r++) {
        double start_time = get_time_sec();
        last_result = dot_product_short(a, b, (int)n);
        double end_time = get_time_sec();
        double elapsed_time = end_time - start_time;
        total += elapsed_time;
    }

    double avg_time = total / REPEAT;
    printf("%-4s | %-12s | %10zu | %12.6f | %lld\n", "C", "short", n, avg_time, last_result);

    if (fp != NULL) {
        fprintf(fp, "C,short,%zu,%.6f\n", n, avg_time);
        fflush(fp);
    }

    free(a);
    free(b);
    a = NULL;
    b = NULL;   
}

void test_signed_char_performance(size_t n, FILE *fp)
{
    signed char *a = (signed char *)malloc(n * sizeof(signed char));
    signed char *b = (signed char *)malloc(n * sizeof(signed char));
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed for signed char arrays of size %zu\n", n);
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++) {
        a[i] = rand() % 201 - 100; // random signed char in range [-100, 100]
        b[i] = rand() % 201 - 100; 
    }
    double total = 0.0;
    long long last_result = 0;
    for (int r = 0; r < REPEAT; r++) {
        double start_time = get_time_sec();
        last_result = dot_product_signed_char(a, b, (int)n);
        double end_time = get_time_sec();
        double elapsed_time = end_time - start_time;
        total += elapsed_time;
    }

    double avg_time = total / REPEAT;
    printf("%-4s | %-12s | %10zu | %12.6f | %lld\n", "C", "signed char", n, avg_time, last_result);

    if (fp != NULL) {
        fprintf(fp, "C,signed char,%zu,%.6f\n", n, avg_time);
        fflush(fp);
    }

    free(a);
    free(b);
    a = NULL;
    b = NULL;
}

double get_time_sec()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

