#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define BUFFER_SIZE 2
#define SHARED_MEMORY_NAME "/my_shared_memory"

int buffer[BUFFER_SIZE];
sem_t *empty, *full, *mutex;
int next_produce = 0;  // Index for the next item to be produced
const int MAX_ITEMS = 5;

void *producer(void *arg) {
    int item = 1;
    while (1) {
        sem_wait(empty);
        sem_wait(mutex);

        // Produce item and add it to the buffer
        buffer[next_produce] = item;
        printf("Produced: %d\n", item);
        next_produce = (next_produce + 1) % BUFFER_SIZE;

        sem_post(mutex);
        sem_post(full);
        sleep(1);  // Simulate work

        if (item == MAX_ITEMS) {
            break;  // Break the loop after producing the last item
        }

        item++;
    }

    pthread_exit(NULL);  // Terminate the thread
}

int main() {
    // Initialize semaphores and shared memory
    empty = sem_open("/my_empty", O_CREAT, 0666, BUFFER_SIZE);
    full = sem_open("/my_full", O_CREAT, 0666, 0);
    mutex = sem_open("/my_mutex", O_CREAT, 0666, 1);

    pthread_t prod_thread;
    pthread_create(&prod_thread, NULL, producer, NULL);

    pthread_join(prod_thread, NULL);

    return 0;
}

