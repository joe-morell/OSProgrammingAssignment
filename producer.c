#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define BUFFER_SIZE 2

int* buffer;
sem_t* mutex;
sem_t* empty;
sem_t* full;
const int MAX_ITEMS = 5;
int items_produced = 0; // Initialize the item counter

void* producer(void* arg) {
    while (1) {
        sem_wait(empty);
        sem_wait(mutex);

        if (items_produced == MAX_ITEMS) {
            // Signal the consumer to exit
            sem_post(mutex);
            sem_post(full);
            break;
        }

        // Critical Section: Add item to the buffer
        buffer[0] = items_produced + 1;
        printf("Produced item %d\n", items_produced + 1);
        items_produced++;

        sem_post(mutex);
        sem_post(full);

        sleep(1);  // Simulate some work
    }
    
    pthread_exit(NULL);  // Terminate the thread
}

int main() {
    // Shared memory setup
    int shm_fd = shm_open("/buffer", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(int) * BUFFER_SIZE);
    buffer = (int*)mmap(NULL, sizeof(int) * BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    mutex = sem_open("/mutex", O_CREAT | O_RDWR, 0666, 1);
    empty = sem_open("/empty", O_CREAT | O_RDWR, 0666, BUFFER_SIZE);
    full = sem_open("/full", O_CREAT | O_RDWR, 0666, 0);

    // Thread setup
    pthread_t producer_thread;
    pthread_create(&producer_thread, NULL, producer, NULL);

    pthread_join(producer_thread, NULL);

    // Cleanup
    munmap(buffer, sizeof(int) * BUFFER_SIZE);
    close(shm_fd);
    sem_unlink("/mutex");
    sem_unlink("/empty");
    sem_unlink("/full");

    return 0;
}

