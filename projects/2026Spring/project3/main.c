#include "sgemm.h"

int isValidInput(int input) {
    if(input <= 0 || input > 100000) {
        return 0;
    }
    else {
        return 1;
    }
}

int main(int argc, char *argv[]) {
    int rows1 = 16, cols1 = 16, cols2 = 16;

    struct Matrix *A, *B, *C_1, *C_2, *C_3;

    if(argc == 2) {
        if (!isValidInput(atoi(argv[1]))) {
            fprintf(stderr, "Invalid input: must be integer between 1 and 100000\n");
            return EXIT_FAILURE;
        }
        rows1 = cols1 = cols2 = isValidInput(atoi(argv[1])) ? atoi(argv[1]) : 16;
    }

    printf("Matrix dimensions: A(%d x %d), B(%d x %d), C(%d x %d)\n", rows1, cols1, cols1, cols2, rows1, cols2);

    A = generate_matrix(rows1, cols1);
    B = generate_matrix(cols1, cols2);
    C_1 = generate_matrix(rows1, cols2);
    C_2 = generate_matrix(rows1, cols2);
    C_3 = generate_matrix(rows1, cols2);

    // sgemm(A, B, C_1, sgemm_plain);
    sgemm(A, B, C_2, sgemm_improved);
    sgemm(A, B, C_3, sgemm_openblas);

    // check_result(C_1, C_2, C_3, rows1, cols2);
    check_result(C_2, C_3, C_3, rows1, cols2);

    freeMatrix(A);
    freeMatrix(B);
    freeMatrix(C_1);
    freeMatrix(C_2);
    freeMatrix(C_3);

    return 0;
}