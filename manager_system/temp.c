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

    if ((sharedMem = mmap(0, shmSize, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0)) == (shm *)-1)
    {
        perror("mmap");
        return 1;
    }
    
    sharedMem->lpr_entrance_1[0] = "1";
    sharedMem->lpr_entrance_1[1] = "2";
    sharedMem->lpr_entrance_1[2] = "3";
    sharedMem->lpr_entrance_1[3] = "A";
    sharedMem->lpr_entrance_1[4] = "B";
    sharedMem->lpr_entrance_1[5] = "C";

    printf("stored rego in lpr: ");
    for (int i = 0; i < 6; i++)
    {
        printf("%s", sharedMem->lpr_entrance_1[i]);
    }
    printf("\n");

    while(1){
        //until program is stopped by user - for testing
    }
    //close
    if (munmap(sharedMem, shmSize) != 0) {
        perror("munmap() failed");
    }

    if (shm_unlink(key) != 0) {
        perror("shm_unlink() failed");
    }

}