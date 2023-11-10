#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 2

int buffer[BUFFER_SIZE];
sem_t *empty, *full, *mutex;
int in = 0;  // Index for the next item to be produced
int out = 0; // Index for the next item to be consumed
const int MAX_ITEMS = 5;

void *consumer(void *arg) {
    while (1) {
        sem_wait(full);
        sem_wait(mutex);

        // Consume item from the buffer
        int item = buffer[out];
        printf("Consumed: %d\n", item);
        out = (out + 1) % BUFFER_SIZE;

        sem_post(mutex);
        sem_post(empty);
        sleep(1);  // Simulate work

        if (item == MAX_ITEMS) {
            break;  // Break the loop after consuming the last item
        }
    }

    pthread_exit(NULL);  // Terminate the thread
}

int main() {
    empty = sem_open("/my_empty", O_CREAT, 0666, BUFFER_SIZE);
    full = sem_open("/my_full", O_CREAT, 0666, 0);
    mutex = sem_open("/my_mutex", O_CREAT, 0666, 1);

    pthread_t cons_thread;
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(cons_thread, NULL);

    return 0;
}

