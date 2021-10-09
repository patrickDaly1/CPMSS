#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


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

int main(){
    //open the shared memory - PARKING

    //setup hash table
    /** Hash table values:
     * Rego (string) (key)
     * timeEntered (clock_t - maybe)
     * levelParked (int)
    */

    //tasks to do - maybe in while loop or they each are treads (no while loop needed) but must wait for them 
    //to finish.

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
}