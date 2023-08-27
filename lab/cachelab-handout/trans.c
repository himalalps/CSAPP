/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"
#include <stdio.h>

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, k, l, x1, x2, x3, x4, x5, x6, x7, x8;

    if (M != N) {
        for (i = 0; i < N - 7; i += 8) {
            for (j = 0; j < M; j++) {
                for (k = 0; k < 8; k++) {
                    B[j][i + k] = A[i + k][j];
                }
            }
        }
        if (!N % 8) {
            return;
        }
        for (j = 0; j < M; j++) {
            for (k = 0; k < N % 8; k++) {
                B[j][N - 1 - k] = A[N - 1 - k][j];
            }
        }
    } else if (M < 64) {
        for (i = 0; i < N - 7; i += 8) {
            for (j = 0; j < M; j++) {
                x1 = A[i][j];
                x2 = A[i + 1][j];
                x3 = A[i + 2][j];
                x4 = A[i + 3][j];
                x5 = A[i + 4][j];
                x6 = A[i + 5][j];
                x7 = A[i + 6][j];
                x8 = A[i + 7][j];
                B[j][i] = x1;
                B[j][i + 1] = x2;
                B[j][i + 2] = x3;
                B[j][i + 3] = x4;
                B[j][i + 4] = x5;
                B[j][i + 5] = x6;
                B[j][i + 6] = x7;
                B[j][i + 7] = x8;
            }
        }
        if (!N % 8) {
            return;
        }
        for (j = 0; j < M; j++) {
            for (k = 0; k < N % 8; k++) {
                B[j][N - 1 - k] = A[N - 1 - k][j];
            }
        }
    } else {
        for (i = 0; i < N - 7; i += 8) {
            for (j = 0; j < M - 7; j += 8) {
                for (k = i; k < i + 4; k++) {
                    x1 = A[k][j];
                    x2 = A[k][j + 1];
                    x3 = A[k][j + 2];
                    x4 = A[k][j + 3];
                    x5 = A[k][j + 4];
                    x6 = A[k][j + 5];
                    x7 = A[k][j + 6];
                    x8 = A[k][j + 7];
                    B[j][k] = x1;
                    B[j + 1][k] = x2;
                    B[j + 2][k] = x3;
                    B[j + 3][k] = x4;
                    B[j][k + 4] = x5;
                    B[j + 1][k + 4] = x6;
                    B[j + 2][k + 4] = x7;
                    B[j + 3][k + 4] = x8;
                }
                for (l = j; l < j + 4; l++) {
                    x1 = A[i + 4][l];
                    x2 = A[i + 5][l];
                    x3 = A[i + 6][l];
                    x4 = A[i + 7][l];
                    x5 = B[l][i + 4];
                    x6 = B[l][i + 5];
                    x7 = B[l][i + 6];
                    x8 = B[l][i + 7];
                    B[l][i + 4] = x1;
                    B[l][i + 5] = x2;
                    B[l][i + 6] = x3;
                    B[l][i + 7] = x4;
                    B[l + 4][i] = x5;
                    B[l + 4][i + 1] = x6;
                    B[l + 4][i + 2] = x7;
                    B[l + 4][i + 3] = x8;
                }
                for (k = i + 4; k < i + 8; k++) {
                    x1 = A[k][j + 4];
                    x2 = A[k][j + 5];
                    x3 = A[k][j + 6];
                    x4 = A[k][j + 7];
                    B[j + 4][k] = x1;
                    B[j + 5][k] = x2;
                    B[j + 6][k] = x3;
                    B[j + 7][k] = x4;
                }
            }
            if (!M % 8) {
                continue;
            }
            for (; j < M; j++) {
                for (k = 0; k < 8; k++) {
                    B[j][i + k] = A[i + k][j];
                }
            }
        }
        if (!N % 8) {
            return;
        }
        for (j = 0; j < M; j++) {
            for (k = 0; k < N % 8; k++) {
                B[j][N - 1 - k] = A[N - 1 - k][j];
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}
