#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "hashtable.h"
#include "sharedMemory.h"
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
    int levelCap[NUM_LEVELS];
    //total capacity
    int totalCap;
    //total billing
    double billing;
};

typedef struct thread_mem thread_mem_t;
struct thread_mem
{
    //main memory
    mem_t *mem;
    //LPR number
    int lprNum;
};

//mutex for thread memory (mem_t)
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

long getTimeMilli(void) {
    long ns; // Nanoseconds
    long s;  // Seconds
    struct timespec spec;

    if(clock_gettime(CLOCK_MONOTONIC, &spec) < 0) {
        //issue setting up time
        perror("clock_gettime\n");
        exit(1);
    }

    s  = spec.tv_sec * 1.0e3; // convert seconds to milliseconds
    ns = spec.tv_nsec / 1.0e6; // convert nanoseconds to milliseconds

    return ns + s;
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
    for(int i = 1; i < NUM_LEVELS; ++i) {
        if(mem->levelCap[lowestLvl] > mem->levelCap[i]) {
            lowestLvl = i;
        }
    }
    //unlock mutex
    pthread_mutex_unlock(&mem_lock);
    return lowestLvl + 1;
}

void bgEntrance(shm *sharedMem, int typeIndex) {
    //printf("Boom gate opening at %d\n", typeIndex);
    usleep(2000);
    //printf("Boom %d status 1: %c\n", typeIndex, sharedMem->entrances[typeIndex].BG.status);
    pthread_mutex_lock(&(sharedMem->entrances[typeIndex].BG.lock));
    sharedMem->entrances[typeIndex].BG.status = 'R';
    pthread_cond_signal(&(sharedMem->entrances[typeIndex].BG.condition));
    pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));    

    // wait for sim to change it to open
    pthread_mutex_lock(&(sharedMem->entrances[typeIndex].BG.lock));
    while (sharedMem->entrances[typeIndex].BG.status != 'O')
        pthread_cond_wait(&(sharedMem->entrances[typeIndex].BG.condition), &(sharedMem->entrances[typeIndex].BG.lock));
    // check changed to right value
    //printf("Boom %d status 2: %c\n", typeIndex, sharedMem->entrances[typeIndex].BG.status);
    if (sharedMem->entrances[typeIndex].BG.status != 'O')
    {
        // Hasn't opened properly
        pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));
        printf("Boom gate %d: ", typeIndex);
        perror("Error raising boom gate for entrance\n");
        //exit(1);
    }
    pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));
    
    // BG opened, wait 5ms?
    usleep(20000);
    // close boom gate
    //printf("Boom %d status 3: %c\n", typeIndex, sharedMem->entrances[typeIndex].BG.status);
    pthread_mutex_lock(&(sharedMem->entrances[typeIndex].BG.lock));
    sharedMem->entrances[typeIndex].BG.status = 'L';
    pthread_cond_signal(&(sharedMem->entrances[typeIndex].BG.condition));
    pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));
    //printf("Boom %d status 3: %c\n", typeIndex, sharedMem->entrances[typeIndex].BG.status);

    // check BG is closed
    pthread_mutex_lock(&(sharedMem->entrances[typeIndex].BG.lock));
    while (sharedMem->entrances[typeIndex].BG.status != 'C')
        pthread_cond_wait(&(sharedMem->entrances[typeIndex].BG.condition), &(sharedMem->entrances[typeIndex].BG.lock));
    if (sharedMem->entrances[typeIndex].BG.status != 'C')
    {
        // Hasn't closed properly
        pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));
        printf("Boom gate %d: ", typeIndex);
        perror("Error closing boom gate for entrance\n");
        exit(1);
    }
    pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));
}

void bgExit(shm *sharedMem, int typeIndex) {
    usleep(2000);
    //printf("Boom %d status 1: %c\n", typeIndex, sharedMem->exits[typeIndex].BG.status);
    pthread_mutex_lock(&(sharedMem->exits[typeIndex].BG.lock));
    sharedMem->exits[typeIndex].BG.status = 'R';
    pthread_cond_signal(&(sharedMem->exits[typeIndex].BG.condition));
    pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));

    
    // wait for sim to change it to open
    pthread_mutex_lock(&(sharedMem->exits[typeIndex].BG.lock));
    while (sharedMem->exits[typeIndex].BG.status != 'O')
        pthread_cond_wait(&(sharedMem->exits[typeIndex].BG.condition), &(sharedMem->exits[typeIndex].BG.lock));
    //printf("Boom %d status 2: %c\n", typeIndex, sharedMem->exits[typeIndex].BG.status);
    // check changed to right value (check this is how condition var works)
    if (sharedMem->exits[typeIndex].BG.status != 'O')
    {
        // Hasn't opened properly
        pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));
        perror("Error raising boom gate for exit\n");
        exit(1);
    }
    pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));
    // BG opened, wait 5ms?
    usleep(20000);
    //printf("Boom %d status 3: %c\n", typeIndex, sharedMem->exits[typeIndex].BG.status);
    // close boom gate
    pthread_mutex_lock(&(sharedMem->exits[typeIndex].BG.lock));
    sharedMem->exits[typeIndex].BG.status = 'L';
    pthread_cond_signal(&(sharedMem->exits[typeIndex].BG.condition));
    pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));
    //printf("Boom %d status 4: %c\n", typeIndex, sharedMem->exits[typeIndex].BG.status);

    // check BG is closed
    pthread_mutex_lock(&(sharedMem->exits[typeIndex].BG.lock));
    while (sharedMem->exits[typeIndex].BG.status != 'C')
        pthread_cond_wait(&(sharedMem->exits[typeIndex].BG.condition), &(sharedMem->exits[typeIndex].BG.lock));
    if (sharedMem->exits[typeIndex].BG.status != 'C')
    {
        // Hasn't opened properly
        pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));
        perror("Error closing boom gate for exit\n");
        exit(1);
    }
    //printf("Boom %d status 5: %c\n", typeIndex, sharedMem->exits[typeIndex].BG.status);
    pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));
}

void boomGateOp(shm *sharedMem, char type, int typeIndex) {
    if(type == 'n') {
        //entrance
        bgEntrance(sharedMem, typeIndex);
    } else if(type == 'x') {
        //exit
        bgExit(sharedMem, typeIndex);
    }
}

void *miniManagerLevel(void *arg) {
    thread_mem_t *info = (thread_mem_t *)arg;
    int lprNum = info->lprNum;
    shm *sharedMem = info->mem->sharedMem;
    while(1) {
        //wait for signal of LPR for this level
        pthread_mutex_lock(&(sharedMem->levels[lprNum].LPR.lock));
        pthread_cond_wait(&(sharedMem->levels[lprNum].LPR.condition), &(sharedMem->levels[lprNum].LPR.lock));
        pthread_mutex_lock(&mem_lock);

        //new car on level - increment level capacity, if was on previous level - decrement capacity
        char regoCpy[7];
        memcpy(regoCpy, sharedMem->levels[lprNum].LPR.rego, 6);
        item_t *car = htab_find(info->mem->h, regoCpy);

        if(car->levelParked == lprNum + 1)
        {
            if (info->mem->levelCap[car->levelParked - 1] != 0){
            //--(info->mem->levelCap[car->levelParked - 1]);
            //info->mem->levelCap[car->levelParked - 1] = 0;
            car->levelParked = 0;
            }
        }
        else 
        {
            ++(info->mem->levelCap[lprNum]);
            car->levelParked = lprNum + 1;
        }
        //unlock mutexes
        pthread_mutex_unlock(&mem_lock);
        pthread_mutex_unlock(&(sharedMem->levels[lprNum].LPR.lock));
    }
    return NULL;
}

double validateBill(void) {
    return 0;
}

void *miniManagerExit(void *arg) {
    thread_mem_t *info = (thread_mem_t *)arg;
    //setup file stream
    FILE *fp = fopen("billing.txt", "a");
    if(fp == NULL) {
        perror("fopen billing.txt\n");
    }
    int lprNum = info->lprNum;
    shm *sharedMem = info->mem->sharedMem;

    sharedMem->exits[lprNum].BG.status = 'C';
    while(1) {
        //Check allocated lpr - use condition variable and mutex before accesing it
        pthread_mutex_lock(&(sharedMem->exits[lprNum].LPR.lock));
        pthread_cond_wait(&(sharedMem->exits[lprNum].LPR.condition), &(sharedMem->exits[lprNum].LPR.lock));
        pthread_mutex_lock(&mem_lock);
        //car leaving, get time spent (current time - time saved)
        char regoCpy[7];
        memcpy(regoCpy, sharedMem->exits[lprNum].LPR.rego, 6);
        item_t *car = htab_find(info->mem->h, regoCpy);
        long timeEntered = car->timeEntered;
        double bill = (getTimeMilli() - timeEntered) * 0.05;
        //save into file
        fprintf(fp, "%s %.2f\n", sharedMem->exits[lprNum].LPR.rego, bill);
        //decrement overall capacity, increase revenue

        --(info->mem->totalCap);
        info->mem->billing += bill;
        //boom gate operation
        //Raise boom gates and unlock thread memory, then relock after opening boom gate
        pthread_mutex_unlock(&mem_lock);
        if (!sharedMem->levels[0].alarm1)
        {
            boomGateOp(sharedMem, 'x', lprNum);
        }
        //unlock shared memory
        pthread_mutex_unlock(&(sharedMem->exits[lprNum].LPR.lock));
    }
    fclose(fp);
    return NULL;
}

void *miniManagerEntrance(void *arg) {
    thread_mem_t *info = (thread_mem_t *)arg;
    //constantly check shared memory entrance LPR for regos
    int lprNum = info->lprNum;
    shm *sharedMem = info->mem->sharedMem;
    pthread_mutex_lock(&mem_lock);

    sharedMem->entrances[lprNum].BG.status = 'C';
    pthread_mutex_unlock(&mem_lock);
    while(1) {
        //Check allocated lpr - use condition variable and mutex before accesing it
        pthread_mutex_lock(&(sharedMem->entrances[lprNum].LPR.lock));
        pthread_cond_wait(&(sharedMem->entrances[lprNum].LPR.condition), &(sharedMem->entrances[lprNum].LPR.lock));
        pthread_mutex_unlock(&(sharedMem->entrances[lprNum].LPR.lock));
        pthread_mutex_lock(&mem_lock);
        //printf("Rego read: %s\n", sharedMem->entrances[lprNum].LPR.rego);
        //printf("%ld", sizeof(sharedMem->entrances[lprNum].LPR.rego) / sizeof(char));
        //printf("Rego found: %s\n", (htab_find(info->mem->h, sharedMem->entrances[lprNum].LPR.rego))->rego);
        //Check if rego in list and if car park full
        //sharedMem->entrances[lprNum].LPR.rego
        //copy string into char[]
        char regoCpy[7];
        memcpy(regoCpy, sharedMem->entrances[lprNum].LPR.rego, 6);
        //printf("Rego copy: %s\n", regoCpy);
        // if(htab_find(info->mem->h, sharedMem->entrances[lprNum].LPR.rego) == NULL) {
        if(htab_find(info->mem->h, regoCpy) == NULL) {
            //doesn't exist in list
            // printf("Setting status to 'X' in: %d\n", lprNum);
            pthread_mutex_lock(&(sharedMem->entrances[lprNum].SIGN.lock));
            sharedMem->entrances[lprNum].SIGN.display = 'X';
            pthread_cond_signal(&(sharedMem->entrances[lprNum].SIGN.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[lprNum].SIGN.lock));
        } else if(info->mem->totalCap >= NUM_CARS_PER_LEVEL * NUM_LEVELS) {
            //full
            pthread_mutex_lock(&(sharedMem->entrances[lprNum].SIGN.lock));
            sharedMem->entrances[lprNum].SIGN.display = 'F';
            pthread_cond_signal(&(sharedMem->entrances[lprNum].SIGN.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[lprNum].SIGN.lock));
        } else if(sharedMem->levels[0].alarm1){
            // do nothing with cars
        }else {
            // printf("Setting status to valid number\n");
            //exists in list and car park not full
            ++(info->mem->totalCap);
            htab_change_time(info->mem->h, regoCpy, getTimeMilli());
            pthread_mutex_lock(&(sharedMem->entrances[lprNum].SIGN.lock));

            pthread_mutex_unlock(&mem_lock);
            sharedMem->entrances[lprNum].SIGN.display = findFreeLevel(info->mem) + '0'; //converts int to char for 0-9
            pthread_cond_signal(&(sharedMem->entrances[lprNum].SIGN.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[lprNum].SIGN.lock));
            
            //Raise boom gates and unlock thread memory, then relock after opening boom gate
            
            boomGateOp(sharedMem, 'n', lprNum);
            pthread_mutex_lock(&mem_lock);
            //increment car park capacity (lpr levels will alter level capacity)
        }
        //unlock rego and car park memory
        
        pthread_mutex_unlock(&mem_lock);
    }
    return NULL;
}

void *displayStatus(void *arg) {
    mem_t *mem = (mem_t *)arg;
    shm *sharedMem = mem->sharedMem;
    while(1) {
        int r = rand()%8;
        
        printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX Status of Car Park XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
        pthread_mutex_lock(&mem_lock);
        //how full each level is
        for (int i = 0; i < NUM_LEVELS; ++i) {
            if (r == 1)
            {
                if (mem->levelCap[i] != 0)
                    --mem->levelCap[i];
            }
            if (i == NUM_LEVELS - 1) {
                printf("| Level %d: %d/%d\n\n", i + 1, mem->levelCap[i], NUM_CARS_PER_LEVEL);
            } else {
                printf("| Level %d: %d/%d ", i + 1, mem->levelCap[i], NUM_CARS_PER_LEVEL);
            }
        }
        //status of LPRs
        printf("\n");
        for (int i = 0; i < NUM_ENTRANCES; ++i) {
            if (i == NUM_ENTRANCES - 1) { 
                printf("| Entrance LPR %d: %s\n\n", i + 1, sharedMem->entrances[i].LPR.rego);
            } else {
                printf("| Entrance LPR %d: %s ", i + 1, sharedMem->entrances[i].LPR.rego);
            }
        }
        for (int i = 0; i < NUM_EXITS; ++i) {
            if (i == NUM_EXITS - 1) {
                printf("| Exit LPR %d: %s\n\n", i + 1, sharedMem->exits[i].LPR.rego);
            } else {
                printf("| Exit LPR %d: %s ", i + 1, sharedMem->exits[i].LPR.rego);
            }       
        }
        for (int i = 0; i < NUM_LEVELS; ++i) {
            if (i == NUM_LEVELS - 1) {
                printf("| Level %d LPR: %s\n\n", i + 1, sharedMem->levels[i].LPR.rego);
            } else {
                printf("| Level %d LPR: %s ", i + 1, sharedMem->levels[i].LPR.rego);
            }
        }
        //status of boom gates
        printf("\n");
        for (int i = 0; i < NUM_ENTRANCES; ++i) {
            if (i == NUM_ENTRANCES - 1) {
                printf("| Entrance BG %d: %c\n\n", i + 1, sharedMem->entrances[i].BG.status);
            } else {
                printf("| Entrance BG %d: %c ", i + 1, sharedMem->entrances[i].BG.status);
            }
        }
        for (int i = 0; i < NUM_EXITS; ++i) {
            if (i == NUM_EXITS - 1) {
                printf("| Exit BG %d: %c\n\n", i + 1, sharedMem->exits[i].BG.status);
            } else {
                printf("| Exit BG %d: %c ", i + 1, sharedMem->exits[i].BG.status);
            }
        }
        //status of signs
        printf("\n");
        for (int i = 0; i < NUM_ENTRANCES; ++i) {
            if (i == NUM_ENTRANCES - 1) {
                printf("| Entrance Sign %d: %c\n\n", i + 1, sharedMem->entrances[i].SIGN.display);
            } else {
                printf("| Entrance Sign %d: %c ", i + 1, sharedMem->entrances[i].SIGN.display);
            }
        }
        //state of temperature sensors
        printf("\n");
        for (int i = 0; i < NUM_LEVELS; ++i) {
            if (i == NUM_LEVELS - 1) {
                printf("| Level %d Temp: %d\n\n", i + 1, sharedMem->levels[i].tempSen1);
            } else {
                printf("| Level %d Temp: %d ", i + 1, sharedMem->levels[i].tempSen1);
            }
        }
        //alarm
        printf("| Alarm: %d\n\n", sharedMem->levels[0].alarm1);
        //revenue
        printf("| Revenue so far: %.2f\n\n", mem->billing);
        pthread_mutex_unlock(&mem_lock);
        //wait 50ms
        fflush(stdout);
        usleep(50000);
        system("clear");
    }
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
    const char *key = "PARKING"; //change to "PARKING"
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
    size_t buckets = 100;
    htab_t h;
    if (!htab_init(&h, buckets))
    {
        perror("failed to initialise hash table\n");
        return 1;
    }
    //Setup file reader
    FILE *fp;
    if((fp = fopen("plates.txt", "r")) == NULL) {
        perror("fopen\n");
        return 1;
    }
    char regos[100][7];
    int index = 0;
    while(fscanf(fp, "%s", regos[index]) != EOF) {
        ++index;
    }
    for(int i = 0; i < 100; ++i) {
        htab_add(&h, regos[i], 0, 0);
    }
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
    //create threads (including status display)
    int total = NUM_ENTRANCES + NUM_EXITS + NUM_LEVELS + 1;
    pthread_t pid[total];
    for(int i = 0; i < total - 1; ++i) {
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
    //create status display thread (pass mem)
    pthread_create(&pid[total - 1], NULL, displayStatus, (void *)info);
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