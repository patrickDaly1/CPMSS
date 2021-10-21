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

#define NUM_ENTRANCES 5
#define NUM_EXITS 5
#define NUM_LEVELS 5
#define NUM_CARS_PER_LEVEL 20

typedef struct mem mem_t;
struct mem
{
    //hashtable pointer
    htab_t *h;
    //shared memory pointer
    shm *sharedMem;
    //level capacity array
    int levelCap[NUM_LEVELS]; //check updates
    //total capacity
    int totalCap; //check updates
    //total billing (cents)
    int billing; //check updates
};

typedef struct thread_mem thread_mem_t;
struct thread_mem
{
    //main memory
    mem_t *mem;
    //LPR number
    int lprNum;
};

//mutex for memory (mem_t)
pthread_mutex_t mem_lock;

/** The Manager 
 * 
 * To do:
 * 
 * Monitor status of LPR sensors - shared memory
 * Keep track of where each car is in carpark - hash table
 * Tell boom gates when to open and close - 
 * Control what's displayed on information signs at entrances
 * Ensure room in carpark before letting more cars in - check manager's dynamic memory
 * Keep track of how full levels are and direct cars to levels that aren't full - check manager's dynamic memory
 * Keep track of time car has been in parking lot and produce bill once leaves (exit LPR)
 * 
 * Display current status of parking lot on frequently updating screen: how full each level is, 
 * current status of boom gates, signs, temperature sensors and alarms and current car park revenue
 */

long getTimeMilli() {
    long ms; // Milliseconds
    long s;  // Seconds
    struct timespec spec;

    if(clock_gettime(CLOCK_MONOTONIC, &spec) < 0) {
        //issue setting up time
        perror("clock_gettime\n");
        exit(1);
    }

    s  = spec.tv_sec * 1.0e3; // convert to milliseconds
    ms = spec.tv_nsec / 1.0e6; // convert nanoseconds to milliseconds

    return ms + s;
}

int findFreeLevel(mem_t *mem) {
    //lock mutex
    pthread_mutex_lock(&mem_lock);
    //Check total capacity
    if(mem->totalCap >= NUM_CARS_PER_LEVEL * NUM_LEVELS) {
        return 0;
    }
    //iterate over capacities to find lowest
    int lowestLvl = 0;
    for(int i = 1; i < NUM_CARS_PER_LEVEL; ++i) {
        if(mem->levelCap[lowestLvl] > mem->levelCap[i]) {
            lowestLvl = i;
        }
    }
    //unlock mutex
    pthread_mutex_unlock(&mem_lock);
    return lowestLvl;
}

void *miniManagerLevel(void *arg) {
    thread_mem_t *info = (thread_mem_t *)arg;

    //check level
    return NULL;
}

void *miniManagerExit(void *arg) {
    thread_mem_t *info = (thread_mem_t *)arg;
    //constantly check shared memory exit LPR for regos (same loop)
    //if new rego in exit LPR: 
    //1. stop park time, calculate billing and save in billing.txt
    //2. decrement previous level capacity, decrement car park capacity, increase revenue
    //3. raise boom gate, wait 20ms, close boom gate
    return NULL;
}

void *miniManagerEntrance(void *arg) {
    thread_mem_t *info = (thread_mem_t *)arg;
    //constantly check shared memory entrance LPR for regos
    // int lprNum = info->lprNum;
    int lprNum = 0;
    shm *sharedMem = info->mem->sharedMem;
    while(1) {
        //Check allocated lpr - use condition variable and mutex before accesing it. Wait for it to change
        pthread_mutex_lock(&(sharedMem->entrances[lprNum].LPR.lock));
        pthread_cond_wait(&(sharedMem->entrances[lprNum].LPR.condition), &(sharedMem->entrances[lprNum].LPR.lock));
        pthread_mutex_unlock(&(sharedMem->entrances[lprNum].LPR.lock));
        //Check if rego in list (and check if aready allocated incase accidentally re-read value)
        if(htab_find(info->mem->h, sharedMem->entrances[lprNum].LPR.rego) == NULL) {
            //doesn't exist in list - set sign to 'x' and signal change
            pthread_mutex_lock(&(sharedMem->entrances[0].SIGN.lock));
            sharedMem->entrances[0].SIGN.display = 'x';
            pthread_mutex_unlock(&(sharedMem->entrances[0].SIGN.lock));

            pthread_mutex_lock(&(sharedMem->entrances[0].SIGN.lock));
            pthread_cond_signal(&(sharedMem->entrances[0].SIGN.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[0].SIGN.lock));
        } else {
            //exists in list
            //If already allocated, ignore
            //Else all good, initialise time entered, find level that a park is free, display level number
            htab_change_time(info->mem->h, sharedMem->entrances[lprNum].LPR.rego, getTimeMilli());

        }
        
        //Raise boom gates for 20ms (check), then close boom gates


    }
    //if new rego in entrance LPR: 
    //0. check if car park full - reject all cars until it isn't full
    //1. if car park not full, check if included in hashmap (if not, reject - display on sign maybe?)
    //2. if in hashmap, enter time in hashmap, display level number with available parks
    //raise boom gate
    //wait 20ms then close boom gate
    return NULL;
}

void *displayStatus(void *arg) {
    //clear display - system("clear")

    //display how full each level

    //current status of boom gates, signs, temperature sesors and alarms (shared memory)

    //revenue car park has brought in ()

    //sleep(50) //50ms sleep
    return NULL;
}

int main(void) {
    //initialise main memory mutex
    if (pthread_mutex_init(&mem_lock, NULL) != 0)
    {
        perror("main memory mutex\n");
        return 1;
    }
    //open the shared memory - key: PARKING (PARKING_TEST for test)
    int shm_fd;
    const char *key = "PARKING_TEST"; //change to "PARKING"
    shm *sharedMem;
    size_t shmSize = 2920;

    //Locate the segment
    if ((shm_fd = shm_open(key, O_RDWR, 0)) < 0)
    {
        perror("shm_open");
        return 1;
    }

    //Attach segment to our data space.
    if ((sharedMem = (shm *)mmap(0, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (void *)-1)
    {
        perror("mmap");
        return 1;
    }

    //setup hashtable
    size_t buckets = 10;
    htab_t h;
    if (!htab_init(&h, buckets))
    {
        perror("failed to initialise hash table\n");
        return 1;
    }

    //Setup file reader
    FILE *fp;
    size_t len = 10;
    char *line;
    size_t read;
    if((fp = fopen("plates.txt", "r")) == NULL) {
        perror("fopen\n");
        return 1;
    }
    if((line = (char *)malloc(len * sizeof(char))) == NULL) {
        perror("Unable to allocate memory for line\n");
        exit(2);
    }

    //Read plates.txt per line and store in hashtable
    while((read = getline(&line, &len, fp)) != -1) { //function
        char copy[read];
        strncpy(copy, line, read - 2);
        htab_add(&h, copy, 0, 0);
    }
    free(line);
    fclose(fp);

    //allocate memory for capacity and billed money (cents) - maybe make struct for this
    mem_t *info = (mem_t *)malloc(sizeof(mem_t)); //function
    info->billing = 0;
    info->totalCap = 0;
    for(int i = 0; i < NUM_LEVELS; ++i) {
        info->levelCap[i] = 0;
    }
    info->h = &h;
    info->sharedMem = sharedMem;
    
    //create threads
    int total = NUM_ENTRANCES + NUM_EXITS + NUM_LEVELS;
    pthread_t pid[total];
    for(int i = 0; i < total; ++i) {
        if(i < NUM_ENTRANCES) {
            //entrance thread
            thread_mem_t *threadInfo = (thread_mem_t *)malloc(sizeof(thread_mem_t));
            threadInfo->mem = info;
            threadInfo->lprNum = i;
            pthread_create(&pid[i], NULL, miniManagerEntrance, (void *)threadInfo);
        } else if (i >= NUM_ENTRANCES && i < NUM_ENTRANCES + NUM_EXITS) {
            //exit thread
            thread_mem_t *threadInfo = (thread_mem_t *)malloc(sizeof(thread_mem_t));
            threadInfo->mem = info;
            threadInfo->lprNum = i - NUM_ENTRANCES;
            pthread_create(&pid[i], NULL, miniManagerExit, (void *)threadInfo);
        } else {
            //level thread
            thread_mem_t *threadInfo = (thread_mem_t *)malloc(sizeof(thread_mem_t));
            threadInfo->mem = info;
            threadInfo->lprNum = i - NUM_ENTRANCES - NUM_EXITS;
            pthread_create(&pid[i], NULL, miniManagerLevel, (void *)threadInfo);
        }
    }
    //wait till threads finish
    for(int i = 0; i < total; ++i) {
        pthread_join(pid[i], NULL);
    }

    //close
    if (munmap(sharedMem, shmSize) != 0) {
        perror("munmap");
    }

    if (shm_unlink(key) != 0) {
        perror("shm_unlink");
    }
    htab_destroy(&h);
    free(info); //check if necessary or does thread do it?
    //free thread info memory - iterate
    return 0;
}