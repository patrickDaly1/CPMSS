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

#define entrys 5;
#define exits 5;
#define levels 5;
#define cars_per_level 20;

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
 * What threads used? A. 1 thread per entramce and exit (might have threads for an exit/entrance incase of
 * differing number of entrances/exits)
 */


//Pass the shared memory and the rego hash table
//A mini manager is what manages at maximum 1 exit and 1 entrance
void miniManagerCreater() {

}

//pass the shared memory and rego hash table
void displayStatus() {

}

//pass shared memory and rego hasth table
void parkingManager() {
    //a single thread that manages car info after they've entered and before tehy exit

    //How to remember if car has entered that level before

    //if car 'enteres a level' (first time LPR has read that car) and there is room on that level - 
    //change car level to that level.

    //'enteres a level' check: 
    //1. if levelParked is less than the current level - parking on this level if room.
    //2. if levelParked is greater than or equal to current level - leaving and don't alter parking space.
    //Do we check if car went to top level and couldn't find a park to came back down?


}

int main() {
    //open the shared memory - PARKING (PARKING_TEST for test)
    int shm_fd;
    const char *key = "PARKING_TEST";
    shm *sharedMem;
    /*
     * Locate the segment.
     */
    if ((shm_fd = shm_open(key, O_RDWR, 0)) < 0)
    {
        perror("shm_open");
        return 1;
    }

    /*
     * Now attach segment to our data space.
     */
    size_t shmSize = 2920;
    if ((sharedMem = mmap(0, shmSize, PROT_WRITE | PROT_READ, MAP_SHARED, shm_fd, 0)) == (shm *)-1)
    {
        perror("mmap");
        return 1;
    }

    //now read what was stored from temp
    printf("stored rego in lpr: ");
    for (int i = 0; i < 6; i++)
    {
        printf("%s", sharedMem->lpr_entrance_1[i]);
    }
    printf("\n");

    
    //setup hash table
    /** Hash table values:
     * Rego (string) (key)
     * timeEntered (clock_t - maybe)
     * levelParked (int)
    */

    //Car park allocation: monotor LPR entrance sensors, when new rego read then check hash
    //table and add time entered then tell boom gate to open for a certain period of time then close again.
    
    //Note: when car parked on a level, if LPR on that level picks up same rego AGAIN - remember it's already 
    //parked so don't add as parked.
    //Note: as car drives through the LPR sensors on ech level, if there is space on a level that it wasn't 
    //told to park - add it as level parked until it finally reaches chooses one. 

    //Car park deallocation: monitor LPR exit sensors, when rego read then the billing info is saved to file and
    //boom gate opens for certain time then closes. 


    //function: creates threads according to number of entrances/exits (account for differening number of 
    //entrances and exits)

    //function: display status of car park constantly (loop with sleep)
    return 0;
}