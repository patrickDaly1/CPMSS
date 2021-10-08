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
 * What threads used? A. 1 for 
 */

int main(){
    //open the shared memory - PARKING

    //Does manager use the hashtable? Might as well - would be useful


    //tasks to do - maybe in while loop or they each are treads (no while loop needed) but must wait for them 
    //to finish.
    //1. Car park allocation: monotor LPR entrance sensors, when new rego read (check validity) then add to hash
    //table (CHECK THIS) with time entered then tell boom gates to open. Note: when car parked on a level, if LPR on that 
    //level picks up same rego AGAIN - remember it's already parked so don't add as parked.
    //When do I tell boom gates to close? A. after car "enters" - LPR read, digital sign level displayed, 

    //2. Car park deallocation: monitor LPR exit sensors, when 

}