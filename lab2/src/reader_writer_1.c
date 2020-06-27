#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t mutex1, mutex2, writePending, readBlock, writeBlock;
int readerCnt = 0;
int writerCnt = 0;
int sharedVar = 0;

void *reader(void *arg) {
    long id = (long)arg;
    //* block readers if there is pending writer
    pthread_mutex_lock(&writePending);
    pthread_mutex_lock(&readBlock);
    pthread_mutex_lock(&mutex1);
    readerCnt ++;
    if (readerCnt == 1) {
        pthread_mutex_lock(&writeBlock);
    }
    pthread_mutex_unlock(&mutex1);
    pthread_mutex_unlock(&readBlock);
    pthread_mutex_unlock(&writePending);

    // reading
    sleep(1);
    printf("Reader %ld is reading shared var %d\n", id, sharedVar);

    pthread_mutex_lock(&mutex1);
    readerCnt --;
    if (readerCnt == 0) {
        pthread_mutex_unlock(&writeBlock);
    }
    pthread_mutex_unlock(&mutex1);

    pthread_exit(NULL);
}

void *writer(void *arg) {
    long id = (long)arg;
    pthread_mutex_lock(&mutex2);
    writerCnt ++;
    if (writerCnt == 1) {
        pthread_mutex_lock(&readBlock);
    }
    pthread_mutex_unlock(&mutex2);

    // writing
    pthread_mutex_lock(&writeBlock);
    sharedVar ++;
    sleep(1);
    printf("Writer %ld has written shared var and it's now %d\n", id, sharedVar);
    pthread_mutex_unlock(&writeBlock);

    pthread_mutex_lock(&mutex2);
    writerCnt--;
    if (writerCnt == 0) {
       pthread_mutex_unlock(&readBlock);
    }
    pthread_mutex_unlock(&mutex2);

    pthread_exit(NULL);
}

int main() {
    pthread_t readers[20];
    pthread_t writers[20];

    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_mutex_init(&readBlock, NULL);
    pthread_mutex_init(&writeBlock, NULL);
    pthread_mutex_init(&writePending, NULL);

    long i;
    for (i = 0; i < 10; i++) {
        pthread_create(&readers[i], NULL, (void*)reader, (void*)i);
    }

    for (i = 0; i < 10; i++) {
        pthread_create(&writers[i], NULL, (void*)writer, (void*)i);
    }

    for (i = 10; i < 20; i++) {
        pthread_create(&readers[i], NULL, (void*)reader, (void*)i);
    }

    for (i = 10; i < 20; i++) {
        pthread_create(&writers[i], NULL, (void*)writer, (void*)i);
    }

    for (i = 0; i < 20; i++) {
        pthread_join(readers[i], NULL);
        pthread_join(writers[i], NULL);
    }

    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);
    pthread_mutex_destroy(&readBlock);
    pthread_mutex_destroy(&writeBlock);
    pthread_mutex_destroy(&writePending);

    pthread_exit(NULL);
}