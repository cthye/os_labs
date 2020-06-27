#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

int shared_var = 0;

void *threadfun(void *threadid) {
    long tid = (long)threadid;
    int times = 50;
    while(times--) {
        shared_var--;
        printf("in thread %ld, shared_var is now %d, in %p\n", tid, shared_var, &shared_var);
        sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[3];
    long t;
    for(t = 0; t < 3; t++) {
        int rc = pthread_create(&threads[t], NULL, threadfun, (void *)t);
        if (rc) {
            printf("error; return code form pthread_create() is %d\n", rc);
            exit(-1);
        }
        sleep(1);
    }

    int times = 100;
    while(times--) {
        shared_var++;
        printf("in main thread, shared_var is now %d in %p\n", shared_var, &shared_var);
    }

    for (t = 0; t < 3; t++)
        pthread_join(threads[t], NULL);
    
    pthread_exit(NULL);
}