#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>

sem_t mutex1, mutex2, readBlock, writeBlock, writePending;
int readerCnt = 0;
int writerCnt = 0;
int sharedVar = 0;


void *reader(void *arg) {
    long id = (long)arg;
    //* block readers if there is pending writer
    sem_wait(&writePending);
    sem_wait(&readBlock);
    sem_wait(&mutex1);
    readerCnt ++;
    if (readerCnt == 1) {
        sem_wait(&writeBlock);
    }
    sem_post(&mutex1);
    sem_post(&readBlock);
    sem_post(&writePending);

    // reading
    printf("Reader %ld begins to read\n", id);
    sleep(1);
    printf("Reader %ld has read shared var %d\n", id, sharedVar);

    sem_wait(&mutex1);
    readerCnt --;
    if (readerCnt == 0) {
        sem_post(&writeBlock);
    }
    sem_post(&mutex1);

    printf("Reader %ld is finished\n", id);

    pthread_exit(NULL);
}

void *writer(void *arg) {
    long id = (long)arg;
    sem_wait(&mutex2);
    writerCnt ++;
    if (writerCnt == 1) {
        sem_wait(&readBlock);
    }
    sem_post(&mutex2);

    // writing
    sem_wait(&writeBlock);
    printf("Writer %ld begins to write\n", id);
    sharedVar ++;
    sleep(1);
    printf("Writer %ld has written shared var and it's now %d\n", id, sharedVar);
    sem_post(&writeBlock);

    sem_wait(&mutex2);
    writerCnt--;
    if (writerCnt == 0) {
       sem_post(&readBlock);
    }
    sem_post(&mutex2);

    printf("Writer %ld is finished\n", id);

    pthread_exit(NULL);
}

int main() {
    pthread_t readers[10];
    pthread_t writers[10];

    sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);
    sem_init(&readBlock, 0, 1);
    sem_init(&writeBlock, 0, 1);
    sem_init(&writePending, 0, 1);

    long i;
    for (i = 0; i < 5; i++) {
        pthread_create(&readers[i], NULL, (void*)reader, (void*)i);
        printf("Reader %ld is created\n", i);
    }
 
    sleep(1);

    for (i = 0; i < 5; i++) {
        pthread_create(&writers[i], NULL, (void*)writer, (void*)i);
	printf("Writer %ld is created\n", i);
    }

    sleep(1);

    for (i = 5; i < 10; i++) {
        pthread_create(&readers[i], NULL, (void*)reader, (void*)i);
        printf("Reader %ld is created\n", i);
    }

    sleep(1);

    for (i = 5; i < 10; i++) {
        pthread_create(&writers[i], NULL, (void*)writer, (void*)i);
        printf("Writer %ld is created\n", i);
    }

    sleep(1);

    for (i = 0; i < 10; i++) {
        pthread_join(readers[i], NULL);
        pthread_join(writers[i], NULL);
    }

    sem_destroy(&mutex1);
    sem_destroy(&mutex2);
    sem_destroy(&readBlock);
    sem_destroy(&writeBlock);
    sem_destroy(&writePending);

    pthread_exit(NULL);
}
