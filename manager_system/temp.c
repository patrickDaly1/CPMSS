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
    int shm_fd;
    shm *sharedMem;
    const char *key = "PARKING_TEST";
    if((shm_fd = shm_open(key, O_CREAT | O_RDWR, 0666)) < 0) {
        perror("shm_open");
        return 1;
    }

    ftruncate(shm_fd, 2920);
    if ((sharedMem = (shm *)mmap(0, 2920, PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (void *)-1)
    {
        perror("mmap");
        return 1;
    }
    
    sharedMem->lpr_entrance_1 = "123ABC";

    printf("stored rego in lpr: %s\n", sharedMem->lpr_entrance_1);
    while(1){
        //until program is stopped by user - for testing
    }
    //close
    if (munmap(sharedMem, 2920) != 0) {
        perror("munmap() failed");
    }

    if (shm_unlink(key) != 0) {
        perror("shm_unlink() failed");
    }

}