#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "hashtable.h"
#include "../shared_memory/sharedMemory.h"
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>


int main() {
    //test hashtable
    // htab_t *htb;
    // size_t buckets = 10;
    // if(!htab_init(htb, buckets)) {
    //     perror("Failed to initialise hash table\n");
    //     return 1;
    // }
    // printf("Got here\n");
    // clock_t fakeTime = 1;
    // htab_add(htb, "fakeRego", fakeTime, 2);
    // htab_add(htb, "fakeRego2", fakeTime, 1);
    // htab_add(htb, "fakeRego3", fakeTime, 3);
    // htab_print(htb);

    //create temporary shared memory
    size_t shmSize = 2920;
    int shm_fd;
    shm *sharedMem;
    const char *key = "PARKING_TEST";

    shm_fd = shm_open(key, O_CREAT | O_RDWR, 0666);
    if(shm_fd < 0) {
        perror("shm_open");
        return 1;
    }

    ftruncate(shm_fd, shmSize);

    if ((sharedMem = (shm *)mmap(0, shmSize, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0)) == (void *)-1)
    {
        perror("mmap");
        return 1;
    }
    strcpy(sharedMem->entrances[0].LPR.rego, "rego");
    printf("%s\n", sharedMem->entrances[0].LPR.rego);

    sharedMem->entrances[0].SIGN.display = 'a';

    //set attributes for mutex and condition variables to PTHREAD_PROCESS_SHARED
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    //initialise mutex and condition variables 
    pthread_mutex_init(&(sharedMem->entrances[0].LPR.lock), &mattr);
    pthread_cond_init(&(sharedMem->entrances[0].LPR.condition), &cattr);
    pthread_mutex_init(&(sharedMem->entrances[0].SIGN.lock), &mattr);
    pthread_cond_init(&(sharedMem->entrances[0].SIGN.condition), &cattr);

    sleep(10);
    strcpy(sharedMem->entrances[0].LPR.rego, "change");
    printf("%s\n", sharedMem->entrances[0].LPR.rego);
    //signal change to rego
    pthread_mutex_lock(&(sharedMem->entrances[0].LPR.lock));
    pthread_cond_signal(&(sharedMem->entrances[0].LPR.condition));
    pthread_mutex_unlock(&(sharedMem->entrances[0].LPR.lock));

    pthread_mutex_lock(&(sharedMem->entrances[0].SIGN.lock));
    pthread_cond_wait(&(sharedMem->entrances[0].SIGN.condition), &(sharedMem->entrances[0].SIGN.lock));
    pthread_mutex_unlock(&(sharedMem->entrances[0].SIGN.lock));

    printf("New sign char: %c\n", sharedMem->entrances[0].SIGN.display);

    sleep(2);
    strcpy(sharedMem->entrances[0].LPR.rego, "931KQD");
    printf("%s\n", sharedMem->entrances[0].LPR.rego);
    pthread_mutex_lock(&(sharedMem->entrances[0].LPR.lock));
    pthread_cond_signal(&(sharedMem->entrances[0].LPR.condition));
    pthread_mutex_unlock(&(sharedMem->entrances[0].LPR.lock));
    
    printf("Here mate\n");
    while(1) {

    }

    //close
    if (munmap(sharedMem, shmSize) != 0) {
        perror("munmap");
    }

    if (shm_unlink(key) != 0) {
        perror("shm_unlink");
    }

    pthread_mutex_destroy(&(sharedMem->entrances[0].LPR.lock));
    pthread_mutexattr_destroy(&mattr);
    
    pthread_cond_destroy(&(sharedMem->entrances[0].LPR.condition));
    pthread_condattr_destroy(&cattr);

    return 0;
}