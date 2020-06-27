#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


int nCustomers;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t Barber = PTHREAD_COND_INITIALIZER;
pthread_cond_t customerArrive;
pthread_cond_t done = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberAvail = PTHREAD_COND_INITIALIZER;
time_t endTime;
pthread_condattr_t attr;


void* barber_method(void* arg)
{
    struct timespec to;
    int retval;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&customerArrive, &attr);
    clock_gettime(CLOCK_MONOTONIC, &to);
    to.tv_sec += 50;

    pthread_mutex_lock(&mutex);
    while (time(NULL) < endTime)
    {
        int status = pthread_cond_timedwait(&customerArrive, &mutex ,&to);
        if (status) 
        {
            //* timeout
            break;
        }
        pthread_cond_signal(&done);
    }
    printf("Barber went home\n");
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void* customer_method(void* arg)
{
    long i = (long)arg;
    pthread_mutex_lock(&mutex);
    printf("customer %ld arrived\n", i);
    if (nCustomers >= 5)
        printf("customer %ld leave\n", i);
    else
    {
        nCustomers++;
        if (nCustomers > 1)
            pthread_cond_wait(&barberAvail, &mutex);
        pthread_cond_signal(&customerArrive);
        printf("customer %ld began\n", i);
        sleep(5);
        pthread_cond_wait(&done, &mutex);
        printf("customer %ld haircut done\n", i);
        pthread_cond_signal(&barberAvail);
        nCustomers--;
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main() {
    pthread_t barber;
    pthread_t customers[10];
   
    endTime = time(NULL)+50;

    pthread_create(&barber, NULL, (void*)barber_method, NULL);
   
    long i;
    for (i = 0; i < 10; i++) {
        sleep(3);
        pthread_create(&customers[i], NULL, (void*)customer_method, (void*)i);
    }

    for (i = 0; i < 10; i++) {
        pthread_join(customers[i], NULL);
    }

    pthread_join(barber, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&customerArrive);
    pthread_cond_destroy(&done);
    pthread_cond_destroy(&barberAvail);

    pthread_exit(NULL);
}