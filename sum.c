/*
 * sum.c
 *
 * CS 470 Project 1 (Pthreads)
 * Serial version
 *
 * Compile with --std=c99
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


// aggregate variables
volatile long sum = 0;
volatile long odd = 0;
volatile long min = INT_MAX;
volatile long max = INT_MIN;
pthread_mutex_t check_sum = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t check_odd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t check_min = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t check_max = PTHREAD_MUTEX_INITIALIZER;

bool done = false;
int thread_count = 0;

// function prototypes
void update(long number);

/*
 * update global aggregate variables given a number
 */
void update(long number)
{
    // simulate computation
    sleep(number);

    // update aggregate variables
    pthread_mutex_lock(&check_sum);
    sum += number;
    pthread_mutex_unlock(&check_sum);
    if (number % 2 == 1) {
        pthread_mutex_lock(&check_odd);
        odd++;
        pthread_mutex_unlock(&check_odd);
    }
    if (number < min) {
        pthread_mutex_lock(&check_min);
        min = number;
        pthread_mutex_unlock(&check_min);
    }
    if (number > max) {
        pthread_mutex_lock(&check_max);
        max = number;
        pthread_mutex_unlock(&check_max);
    }
}

int main(int argc, char* argv[])
{
    // check and parse command line options
    if (argc != 3) {
        printf("Usage: sum <infile> <worker threads>\n");
        exit(EXIT_FAILURE);
    }
    thread_count = strtol(argv[2], NULL, 10);
    if (thread_count < 1) {
        printf("At least 1 worker thread needed");
        exit(EXIT_FAILURE);
    }
    char *fn = argv[1];

    // open input file
    FILE* fin = fopen(fn, "r");
    if (!fin) {
        printf("ERROR: Could not open %s\n", fn);
        exit(EXIT_FAILURE);
    }

    // load numbers and add them to the queue
    char action;
    long num;
    while (fscanf(fin, "%c %ld\n", &action, &num) == 2) {

        // check for invalid action parameters
        if (num < 1) {
            printf("ERROR: Invalid action parameter: %ld\n", num);
            exit(EXIT_FAILURE);
        }

        if (action == 'p') {            // process
            update(num);
        } else if (action == 'w') {     // wait
            sleep(num);
        } else {
            printf("ERROR: Unrecognized action: '%c'\n", action);
            exit(EXIT_FAILURE);
        }
    }
    fclose(fin);

    // print results
    printf("%ld %ld %ld %ld\n", sum, odd, min, max);

    // clean up and return
    return (EXIT_SUCCESS);
}
