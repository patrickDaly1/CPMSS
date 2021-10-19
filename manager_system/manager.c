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

typedef struct thread_mem thread_mem_t;
struct thread_mem
{
    //hashtable pointer
    htab_t *h;
    //shared memory pointer
    shm *sharedMem;
    //level capacity array
    int levelCap[NUM_LEVELS];
    //total capacity
    int totalCap;
    //total billing (cents)
    int billing;
};

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

/**
 * What threads used? A. 1 thread per entrance and exit (might have threads for an exit/entrance incase of
 * differing number of entrances/exits)
 */


//Pass the shared memory and the rego hash table
//A mini manager is what manages at maximum 1 exit and 1 entrance
void *miniManagerCreater(void *arg) {
    //thread function that checks the LPR sensors at an entrance and exit (either one)
    //Will need the hasthable and shared memory

    //constantly check shared memory entrance LPR for regos - busy loop (with sleep) or what?
    //if new rego in entrance LPR: 
    //0. check if car park full - reject all cars until it isn't full
    //1. if car park not full, check if included in hashmap (if not, reject - display on sign maybe?)
    //2. if in hashmap, enter time in hashmap, display level number with available parks
    //raise boom gate
    //wait 20ms then close boom gate

    //constantly check shared memory exit LPR for regos (same loop)
    //if new rego in exit LPR: 
    //1. stop park time, calculate billing and save in billing.txt
    //2. decrement previous level capacity, decrement car park capacity, increase revenue
    //3. raise boom gate, wait 20ms, close boom gate

    //maybe sleep
    return NULL;
}

//pass shared memory and rego hasth table
void parkingManager() {
    //a single thread that manages car info after they've entered and before they exit
    
    //levelParked just means level currently on (might be passing by)
    
    //if car enters a level, increment number of cars on level (even if full or more - can be assumed that 
    //remaining are passing by looking for parks), decrease cars on previous level (even if full or more) 
    //then allocate car to current level.


}

//pass the shared memory (maybe) and hash table
void displayStatus() { //displayed every 50ms
    //clear display - system("clear")

    //display how full each level

    //current status of boom gates, signs, temperature sesors and alarms (shared memory)

    //revenue car park has brought in ()
}

    //Car park allocation: monotor LPR entrance sensors, when new rego read then check hash
    //table and add time entered then tell boom gate to open for a certain period of time then close again.
    
    //Note: when car parked on a level, if LPR on that level picks up same rego AGAIN - remember it's already 
    //parked so don't add as parked.
    //Note: as car drives through the LPR sensors on each level, if there is space on a level that it wasn't 
    //told to park - add it as level parked until it finally chooses one. 

    //Car park deallocation: monitor LPR exit sensors, when rego read then the billing info is saved to file and
    //boom gate opens for certain time then closes. 

    //function: creates threads according to number of entrances/exits (account for differening number of 
    //entrances and exits)

    //function: display status of car park constantly (loop with sleep)

int main(void) {
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
        perror("Unable to allocate buffer\n");
        exit(2);
    }

    //Read plates.txt per line and store in hashtable
    int level = 1;
    while((read = getline(&line, &len, fp)) != -1) {
        char copy[read];
        strncpy(copy, line, read - 2);
        htab_add(&h, copy, 0, level);
        if(level >= 5) {    
            level = 0;
        } else {
            ++level;
        }
    }
    free(line);
    fclose(fp);
    htab_print(&h);

    //allocate memory for capacity and billed money (cents) - maybe make struct for this
    thread_mem_t *threadInfo = (thread_mem_t *)malloc(sizeof(thread_mem_t));

    //close
    if (munmap(sharedMem, shmSize) != 0) {
        perror("munmap");
    }

    if (shm_unlink(key) != 0) {
        perror("shm_unlink");
    }
    free(threadInfo);
    return 0;
}