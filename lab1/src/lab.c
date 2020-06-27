#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

int shared_var = 0;

void *threadfun(void *threadid) {
    int times = 50;
    while(times--) {
        shared_var--;
        printf("in child thread, shared_var is now %d in %p\n", shared_var, &shared_var);
        sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    int rc = pthread_create(&thread, NULL, threadfun, NULL);
    if (rc) {
        printf("error; return code form pthread_create() is %d\n", rc);
        exit(-1);
    }

    int times = 50;
    while(times--) {
        shared_var++;
        printf("in main thread, shared_var is now %d in %p\n", shared_var, &shared_var);
        sleep(1);
    }

    pthread_join(thread, NULL);
    pthread_exit(NULL);
}