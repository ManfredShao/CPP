#include "sgemm.h"
#include <sys/time.h>

double HPL_timer_walltime() {
    struct timeval tp;
    static long long start_sec = 0;
    static long long start_usec = 0;

    if(!start_sec) {
        gettimeofday(&tp, NULL);
        start_sec = tp.tv_sec;
        start_usec = tp.tv_usec;
    }

    gettimeofday(&tp, NULL);
    return (double)(tp.tv_sec - start_sec) + (double)(tp.tv_usec - start_usec) * 1e-6;
}