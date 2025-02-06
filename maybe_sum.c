/*
 * sum.c
 *
 * CS 470 Project 1 (Pthreads)
 * Parallel version
 *
 * Compile with --std=c99 -pthread
 */

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// Task structure
typedef struct {
    char action;
    long number;
} Task;

// Node structure for the task queue
typedef struct Node {
    Task task;
    struct Node* next;
} Node;

// Queue structure
typedef struct {
    Node* head;
    Node* tail;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Queue;

// Global variables
long sum = 0;
long odd = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;
Queue taskQueue;
pthread_mutex_t aggregateMutex = PTHREAD_MUTEX_INITIALIZER;

// Function prototypes
void update(long number);
void* worker(void* arg);
void enqueue(Queue* queue, Task task);
Task dequeue(Queue* queue);

/*
 * Update global aggregate variables given a number
 */
void update(long number) {
    // Simulate computation
    sleep(number);

    // Update aggregate variables
    pthread_mutex_lock(&aggregateMutex);
    sum += number;
    if (number % 2 == 1) {
        odd++;
    }
    if (number < min) {
        min = number;
    }
    if (number > max) {
        max = number;
    }
    pthread_mutex_unlock(&aggregateMutex);
}

/*
 * Worker thread function
 */
void* worker(void* arg) {
    while (true) {
        pthread_mutex_lock(&taskQueue.mutex);
        while (taskQueue.size == 0 && !done) {
            pthread_cond_wait(&taskQueue.cond, &taskQueue.mutex);
        }
        if (taskQueue.size == 0 && done) {
            pthread_mutex_unlock(&taskQueue.mutex);
            break;
        }
        Task task = dequeue(&taskQueue);
        pthread_mutex_unlock(&taskQueue.mutex);

        if (task.action == 'p') {
            update(task.number);
        }
    }
    return NULL;
}

/*
 * Enqueue a task into the queue
 */
void enqueue(Queue* queue, Task task) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->task = task;
    newNode->next = NULL;

    pthread_mutex_lock(&queue->mutex);
    if (queue->tail == NULL) {
        queue->head = newNode;
    } else {
        queue->tail->next = newNode;
    }
    queue->tail = newNode;
    queue->size++;
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

/*
 * Dequeue a task from the queue
 */
Task dequeue(Queue* queue) {
    Node* temp = queue->head;
    Task task = temp->task;

    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    queue->size--;
    free(temp);

    return task;
}

int main(int argc, char* argv[]) {
    // Check and parse command line options
    if (argc != 3) {
        printf("Usage: sum <infile> <num_workers>\n");
        exit(EXIT_FAILURE);
    }
    char* fn = argv[1];
    int numWorkers = atoi(argv[2]);
    if (numWorkers <= 0) {
        printf("ERROR: Number of workers must be greater than 0\n");
        exit(EXIT_FAILURE);
    }

    // Initialize task queue
    taskQueue.head = NULL;
    taskQueue.tail = NULL;
    taskQueue.size = 0;
    pthread_mutex_init(&taskQueue.mutex, NULL);
    pthread_cond_init(&taskQueue.cond, NULL);

    // Create worker threads
    pthread_t workers[numWorkers];
    for (int i = 0; i < numWorkers; i++) {
        pthread_create(&workers[i], NULL, worker, NULL);
    }

    // Open input file
    FILE* fin = fopen(fn, "r");
    if (!fin) {
        printf("ERROR: Could not open %s\n", fn);
        exit(EXIT_FAILURE);
    }

    // Load numbers and add them to the queue
    char action;
    long num;
    while (fscanf(fin, "%c %ld\n", &action, &num) == 2) {
        // Check for invalid action parameters
        if (num < 1) {
            printf("ERROR: Invalid action parameter: %ld\n", num);
            exit(EXIT_FAILURE);
        }

        if (action == 'p') {            // Process
            Task task = {action, num};
            enqueue(&taskQueue, task);
        } else if (action == 'w') {     // Wait
            sleep(num);
        } else {
            printf("ERROR: Unrecognized action: '%c'\n", action);
            exit(EXIT_FAILURE);
        }
    }
    fclose(fin);

    // Signal workers that no more tasks are coming
    pthread_mutex_lock(&taskQueue.mutex);
    done = true;
    pthread_cond_broadcast(&taskQueue.cond);
    pthread_mutex_unlock(&taskQueue.mutex);

    // Wait for all worker threads to finish
    for (int i = 0; i < numWorkers; i++) {
        pthread_join(workers[i], NULL);
    }

    // Print results
    printf("%ld %ld %ld %ld\n", sum, odd, min, max);

    // Clean up and return
    pthread_mutex_destroy(&taskQueue.mutex);
    pthread_cond_destroy(&taskQueue.cond);
    return EXIT_SUCCESS;
}

// works alright when I test times