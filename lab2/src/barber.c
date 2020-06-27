#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

time_t endTime;

sem_t customerArrive, barberAvail, done, mutex;
int chairNum = 5;
int slept = 0;

void *barberShop(void* arg) {
    while(time(NULL) < endTime) {
        int status = sem_trywait(&customerArrive);
        if (!status) {
            slept = 0;
            sem_post(&barberAvail);
            sem_wait(&mutex);
            chairNum++;
            sem_post(&mutex);

            // do cutting
            sleep(5);
            sem_post(&done);
        } else {
            if(!slept) {
                printf("barber is sleeping\n");
                slept = 1;
            }
        }
        // sem_wait(&customerArrive);
        // sem_post(&barberAvail);
        // sem_wait(&mutex);
        // chairNum++;
        // sem_post(&mutex);

        // sleep(5);
        // sem_post(&done);
        
    }
    pthread_exit(NULL);
}

void *customer(void* arg) {
    long id = (long)arg;
    printf("customer %ld arrived\n", id);
    sem_wait(&mutex);
    if (!chairNum) {
        printf("customer %ld leaved\n", id);
        sem_post(&mutex);
    } else {
        chairNum--;
        sem_post(&mutex);

        //? may post several times 
        sem_post(&customerArrive);
        sem_wait(&barberAvail);

        printf("Customer %ld began\n", id);
        // do cutting
        sem_wait(&done);
        printf("customer %ld haircut done\n", id);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t barber;
    pthread_t customers[10];

    sem_init(&customerArrive, 0, 0);
    sem_init(&barberAvail, 0, 0);
    sem_init(&mutex, 0, 1);

    endTime = time(NULL)+40;
   
    pthread_create(&barber, NULL, (void*)barberShop, NULL);
   
    long i;
    for (i = 0; i < 10; i++) {
        sleep(1);
        pthread_create(&customers[i], NULL, (void*)customer, (void*)i);
    }

    for (i = 0; i < 10; i++) {
        pthread_join(customers[i], NULL);
    }

    pthread_join(barber, NULL);

    printf("Barber went home\n");

    sem_destroy(&mutex);
    sem_destroy(&barberAvail);
    sem_destroy(&customerArrive);

    pthread_exit(NULL);
}