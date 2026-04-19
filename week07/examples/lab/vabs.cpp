#include "vabs.h"

using namespace std;

bool vabs (int *p, size_t n) {
    if (p == NULL) {
        return false;
    }
    for (size_t i=0; i<n;i++) {
        if (p[i] < 0) {
            p[i] = -p[i];
        }
    }
    return true;
}

bool vabs (float *p, size_t n) {
    if (p == NULL) {
        return false;
    }
    for (size_t i=0; i<n;i++) {
        if (p[i] < 0) {
            p[i] = -p[i];
        }
    }
    return true;
}

bool vabs (double *p, size_t n) {
    if (p == NULL) {
        return false;
    }
    for (size_t i=0; i<n;i++) {
        if (p[i] < 0) {
            p[i] = -p[i];
        }
    }
    return true;
}
