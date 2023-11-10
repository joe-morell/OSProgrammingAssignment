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
int items_consumed = 0; // Initialize the item counter

void* consumer(void* arg) {
    while (1) {
        sem_wait(full);
        sem_wait(mutex);

        if (items_consumed == MAX_ITEMS) {
            // Signal the producer to exit
            sem_post(mutex);
            sem_post(empty);
            break;
        }

        // Critical Section: Consume item from the buffer
        int item = buffer[0];
        printf("Consumed item %d\n", item);
        items_consumed++;

        sem_post(mutex);
        sem_post(empty);

        sleep(1);  // Simulate some work
    }
    
    pthread_exit(NULL);  // Terminate the thread
}

int main() {
    // Shared memory setup (same as in producer)
    int shm_fd = shm_open("/buffer", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(int) * BUFFER_SIZE);
    buffer = (int*)mmap(NULL, sizeof(int) * BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    mutex = sem_open("/mutex", O_CREAT | O_RDWR, 0666, 1);
    empty = sem_open("/empty", O_CREAT | O_RDWR, 0666, BUFFER_SIZE);
    full = sem_open("/full", O_CREAT | O_RDWR, 0666, 0);

    // Thread setup (or use separate processes if required)
    pthread_t consumer_thread;
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(consumer_thread, NULL);

    // Cleanup (same as in producer)
    munmap(buffer, sizeof(int) * BUFFER_SIZE);
    close(shm_fd);
    sem_unlink("/mutex");
    sem_unlink("/empty");
    sem_unlink("/full");

    return 0;
}

