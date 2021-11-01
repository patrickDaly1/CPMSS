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

/**
 * Returns the current monotonic time in milliseconds (time since some chosen epoch).
 * 
 * Returns: (long) a long integer of the current time in milliseconds. 
 */
long getTimeMilli(void) {
    // nano seconds
    long ns;
    // milli seconds
    long s;
    struct timespec spec;

    if(clock_gettime(CLOCK_MONOTONIC, &spec) < 0) {
        //issue setting up time
        perror("clock_gettime\n");
        exit(1);
    }
    // convert to milliseconds
    s  = spec.tv_sec * 1.0e3;
    ns = spec.tv_nsec / 1.0e6;

    return ns + s;
}

/**
 * Finds any level with the lowest number of cars.
 * 
 * Returns: (int) an integer of an available level.
 */
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

/**
 * Performs the boom gate operation for the entrance. This includes 
 * Raising the boom gate, waiting for the simulator to repsond with
 * the boom gate open, then waiting for the car to pass, then closing 
 * the boom gate and waiting for the simulator to confirm it being closed. 
 * 
 * Returns: (void) nothing is returned. 
 */
void bgEntrance(shm *sharedMem, int typeIndex) {
    usleep(2000);
    pthread_mutex_lock(&(sharedMem->entrances[typeIndex].BG.lock));
    sharedMem->entrances[typeIndex].BG.status = 'R';
    pthread_cond_signal(&(sharedMem->entrances[typeIndex].BG.condition));
    pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));    
    // wait for sim to change it to open
    pthread_mutex_lock(&(sharedMem->entrances[typeIndex].BG.lock));
    while (sharedMem->entrances[typeIndex].BG.status != 'O')
        pthread_cond_wait(&(sharedMem->entrances[typeIndex].BG.condition), &(sharedMem->entrances[typeIndex].BG.lock));
    // check changed to right value
    if (sharedMem->entrances[typeIndex].BG.status != 'O')
    {
        // Hasn't opened properly
        pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));
        printf("Boom gate %d: ", typeIndex);
        perror("Error raising boom gate for entrance\n");
    }
    pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));
    // BG opened, wait 5ms?
    usleep(20000);
    // close boom gate
    pthread_mutex_lock(&(sharedMem->entrances[typeIndex].BG.lock));
    sharedMem->entrances[typeIndex].BG.status = 'L';
    pthread_cond_signal(&(sharedMem->entrances[typeIndex].BG.condition));
    pthread_mutex_unlock(&(sharedMem->entrances[typeIndex].BG.lock));
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

/**
 * Performs the boom gate operation for the exit. This includes 
 * Raising the boom gate, waiting for the simulator to repsond with
 * the boom gate open, then waiting for the car to pass, then closing 
 * the boom gate and waiting for the simulator to confirm it being closed. 
 * 
 * Returns: (void) nothing is returned. 
 */
void bgExit(shm *sharedMem, int typeIndex) {
    usleep(2000);
    pthread_mutex_lock(&(sharedMem->exits[typeIndex].BG.lock));
    sharedMem->exits[typeIndex].BG.status = 'R';
    pthread_cond_signal(&(sharedMem->exits[typeIndex].BG.condition));
    pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));
    
    // wait for sim to change it to open
    pthread_mutex_lock(&(sharedMem->exits[typeIndex].BG.lock));
    while (sharedMem->exits[typeIndex].BG.status != 'O')
        pthread_cond_wait(&(sharedMem->exits[typeIndex].BG.condition), &(sharedMem->exits[typeIndex].BG.lock));
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
    // close boom gate
    pthread_mutex_lock(&(sharedMem->exits[typeIndex].BG.lock));
    sharedMem->exits[typeIndex].BG.status = 'L';
    pthread_cond_signal(&(sharedMem->exits[typeIndex].BG.condition));
    pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));
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
    pthread_mutex_unlock(&(sharedMem->exits[typeIndex].BG.lock));
}

/**
 * Determines which boom gate operation to perform by the given parameter (type) - exit or entrance.
 * 
 * Returns: (void) nothing is returned. 
 */
void boomGateOp(shm *sharedMem, char type, int typeIndex) {
    if(type == 'n') {
        //entrance
        bgEntrance(sharedMem, typeIndex);
    } else if(type == 'x') {
        //exit
        bgExit(sharedMem, typeIndex);
    }
}

/**
 * This function is a thread function that manages a particular level. It monitors the shared memory 
 * and waits for a new registration to come through. When this happens, it checks that the registration is in 
 * the hashtable, changes the stored cars level and alters the level capacity accordingly.
 * 
 * returns: (void *) NULL is returned, nothing important is needed for the return. This part fo the program is designed
 * for a real life car park, so ideally it goes on forever. 
 */
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

/**
 * This function is a thread function that manages a particular exit. It sets up the billing.txt file 
 * and waits for a new registration number to appear. Once this happens, the car is searched in the 
 * hashtable, the billing is calculated and entered into the billing.txt file, the overall car capacity
 * is decreased and the exit boom agte operation is initiated so that the car can leave. 
 * 
 * Returns: (void *) NULL is returned, as nothing important is needed if this function finishes. This part
 * of the program is ideally suppose to last forever to mimic that of a real car park system. 
 */
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

/**
 * This functions is a thread function that deals with a particular entrance. It opens the shared memory
 * and awaits for a new car registration to appear. Once this happens, the registration is checked inside the
 * hashtable, if it's not in the hashtable, the display sign is set to 'X'; if the car park is full, the display
 * sign is set to 'F'. Otherwise, the car is accepted and a free level is displayed in the status display and the
 * boom gate operation for the entrance is triggered.
 * 
 * returns: (void *) NULL is returned as nothing important is expected on the return. This part
 * of the program is ideally suppose to last forever to mimic that of a real car park system. 
 */
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
        char regoCpy[7];
        memcpy(regoCpy, sharedMem->entrances[lprNum].LPR.rego, 6);
        if(htab_find(info->mem->h, regoCpy) == NULL) {
            //doesn't exist in list
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
            //exists in list and car park not full
            ++(info->mem->totalCap);
            htab_change_time(info->mem->h, regoCpy, getTimeMilli());
            pthread_mutex_lock(&(sharedMem->entrances[lprNum].SIGN.lock));
            pthread_mutex_unlock(&mem_lock);
            sharedMem->entrances[lprNum].SIGN.display = findFreeLevel(info->mem) + '0';
            pthread_cond_signal(&(sharedMem->entrances[lprNum].SIGN.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[lprNum].SIGN.lock));
            //Raise boom gates and unlock thread memory, then relock after opening boom gate
            boomGateOp(sharedMem, 'n', lprNum);
            pthread_mutex_lock(&mem_lock);
        }
        //unlock car park memory
        pthread_mutex_unlock(&mem_lock);
    }
    return NULL;
}

/**
 * This function displays all the relevant data to the terminal then clears 
 * is all after 50 milliseconds. The information displayed includes thes state 
 * of the LPRS, the level capacities, the the boom gates, the display signs, the 
 * state of the temperature sensors, the alarm status and the car park revenue. 
 * 
 * Returns: (void *) NULL is returned as nothign valuable is needed on the return. Ideally, this
 * thread function lasts for as long as a realistic car park should last - forever. 
 */
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


/**
 * This is the main function that initialises everything and frees all the necessary memory. It
 * initialises the thread-shared memory, the hashtable, opens the shared memory, creates all the level threads,
 * entrance threads and exit threads then frees all the necessary memory if completion of the program ever occurs.
 * Since the spec didn't specify a means of ending the program, this wasn't necessary - ideally car park programs last 
 * a long time. 
 * 
 * Returns: (int) the status of the program, returns 0 if no problems occured, 1 if they did. 
 */
int main(void) {
    //initialise main memory mutex
    if (pthread_mutex_init(&mem_lock, NULL) != 0)
    {
        perror("main memory mutex\n");
        return 1;
    }
    //open the shared memory - key: PARKING (PARKING_TEST for test)
    int shm_fd;
    const char *key = "PARKING";
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
    free(info);
    return 0;
}