#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>  
#include "linkedlist.h"
#include "../shared_memory/sharedMemory.h"

#define entrys_exits 5;
#define levels 5;
#define cars_per_level 20;

void *temp_sensor(void);
car_t *car_init(queue_t entry_queue);
void car_queuer(queue_t entry_queue);

int main(int argc, char argv)
{    
    /* OPEN SHARED MEMORY */
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

    /* INITIALISE CAR GENERATING THREAD */


    /* INITIALISE BOOM GATE THREADS */

    /* INITIALISE TEMP SENSOR THREADS */
    const int THREADS = 2;
	
	pthread_t threads[THREADS];

    pthread_create(&threads[1], NULL, temp_sensor(), NULL);
    pthread_create(&threads[2], NULL, temp_sensor2(), NULL);


    /* CLOSE SHARED MEMORY */
    if (munmap(sharedMem, shmSize) != 0) {
        perror("munmap");
    }

    if (shm_unlink(key) != 0) {
        perror("shm_unlink");
    }
}

/**
 * Creates new car to add to queue
 * 
 * 1. generate random rego (ie. 123ABC) 
 *  a. ensure this rego is not currently assigned to a car *** FIGURE OUT HOW TO DO THIS ***
 * 2. assign random enterence for car to go to
 */
car_t *car_init(queue_t *entry_queue)
{
    // create new car structure
    car_t *new_car = (car_t *)malloc(sizeof(car_t));

    // generate random rego values
    int odds = rand() % 4;

    // create random probability of guarenteed entry
    new_car->entry = rand() % entrys_exits;
    for (;;)
    {
        if(odds == 0)
        {
            // choose from plates.txt file
        }
        else
        {
            for(int i = 0; i < 3; i++)
            {
                new_car->rego[i] = (char)((rand() % 26) + 65);
                new_car->rego[i + 3] = (char)(rand() % 10);
            }
        }

        // check if car is already in list
        if (!(find_car(entry_queue, new_car->rego)))
        {
            return new_car;
        }
    }
}


/**
 * Function responsible for managing adding new cars into the system
 * 
 * 1. create new car and add to queue
 * 2. sleep for 1 - 100ms
 */
void *car_queuer(queue_t *entry_queue)
{
    for(;;)
    {
        // initialise new car and add to queue
        addCar(entry_queue, car_init(entry_queue));
        // sleep for 1 - 100 ms
        usleep(((rand() % 100) + 1) * 1000);    
    }
}

/** 
 * Function responsible for managing the entry of new cars
 * 
 * 1. sleep thread for (2ms) 
 * 2. find first car from queue that corresponds to correct enterence. 
 * 3. read rego from car and update lpr for corresponding enterence and wait for manager response
 *  a. if denied remove from queue
 *  b. if allowed proceed with below
 * 4. sleep for 10ms (boom gate opening)
 * 5. create new car thread and remove from queue
 * 6. sleep 10ms (boom gate closing)
 * 7. loop to top        
 */
void *boom_gate(queue_t *entry_queue)
{
    // loop infinately
    for(;;)
    {
        usleep(2000); // wait for car
        
        usleep(10000); // open boom gate

        /* CREATE NEW CAR THREAD */

        usleep(10000); // close boom gate
    }
}

/**
 * Function to manage the simulation of car within carpark
 * 
 * 1. sleep for 10ms (traveling to parking)
 * 2. trigger LPR for corresponding level
 * 3. sleep for 100 - 1000 ms
 * 4. trigger level lpr on exit
 * 5. sleep for 10ms (traveling to random exit)
 * 6. trigger corresponding exit LRP
 */
void *car_movement(car_t car_data)
{
    usleep(10000); // travel to parking
 
    usleep(((rand() % 900) + 101) * 1000);
}

/**
 * Function to simulate temperature sensor
 * 
 * 1. every 1 - 5 seconds update sensor with new temp
 */
void *temp_sensor(void)
{
    for(int i = 0; i < 50; i++)
    {
        // update temp global value here
        printf("%d from 1\n", (rand() % 4) + 21);
        usleep(((rand() % 5) + 20) * 1000);
    }
}
void *temp_sensor2(void)
{
    for(int i = 0; i < 50; i++)
    {
        // update temp global value here
        printf("%d from 2\n", (rand() % 4) + 21);
        usleep(((rand() % 5) + 20) * 1000);
    }
}

