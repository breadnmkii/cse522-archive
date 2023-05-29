/*
 * CSE 522S Studio 10
 * Author: Brian Kocoloski
 * 
 * build with:
 * gcc gdb-matrix.c gdb-thread.o -o prog -lpthread
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <assert.h>

#define DELAY_MS    1
#define DELAY_US    (DELAY_MS * 1000)

/* gdb helper vars */
static int * watch_addr_failcell = 0;

/* all defined in a separate file */
extern void * thread_fn(void * arg);
extern int NR_ROWS;
extern int NR_COLS;

static void
matrix_fn(int ** matrix)
{
    int x, y;

    for (y = 0; y < NR_COLS; y++) {
        for (x = 0; x < NR_ROWS; x++) {
            printf("matrix[%d][%d] = %d\n", x, y, matrix[x][y]);
            assert(matrix[x][y] == x*y);
            usleep(DELAY_US);
             
        }
    }
}

int main()
{
    pthread_t child;
    int ** matrix;
    int ret, x, y;

    matrix = malloc(sizeof(int *) * NR_ROWS);
    if (matrix == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (x = 0; x < NR_ROWS; x++) {
        matrix[x] = malloc(sizeof(int) * NR_COLS);
        if (matrix[x] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    } 

    for (x = 0; x < NR_ROWS; x++) {
        for (y = 0; y < NR_COLS; y++) {
            matrix[x][y] = x*y;
        }
    }

    // printf("Cell addr: %p\n", (void *)&matrix[67][3]);
    watch_addr_failcell = &matrix[67][3];

    ret = pthread_create(&child, NULL, thread_fn, matrix);
    if (ret != 0) {
        perror("pthread_create\n");
        exit(EXIT_FAILURE);
    }
    
    matrix_fn(matrix);

    printf("Program completed!\n");
    exit(EXIT_SUCCESS);
}
