#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "linkedlist.h"

#define entrys_exits 5;
#define levels 5;
#define cars_per_level 20;

queue_t *entry_queue = createQueue();
queue_t *incarpark_queue = createQueue();
queue_t *exit_queue = createQueue();

car_t *car_init(void);
void *car_queuer(void);

int main(int argc, char argv)
{
    pthread_t car_init_thread;

    pthread_create(car_init_thread, NULL, car_init, NULL);

    pthread_join(car_init_thread, NULL);
}

/**
 * Creates new car to add to queue
 * 
 * 1. generate random rego (ie. 123ABC) 
 *  a. ensure this rego is not currently assigned to a car *** FIGURE OUT HOW TO DO THIS ***
 * 2. assign random enterence for car to go to
 */
car_t *car_init(void)
{
    // create new car structure
    car_t *new_car = (car_t *)malloc(sizeof(car_t));

    // generate random rego values
    int odds = rand() % 4;

    // create random probability of guarenteed entry
    new_car->entry = rand() % entrys_exits;
    for (;;)
    {
        if(odds == -1)
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
        if (!(find_car(entry_queue, new_car->rego)) || !(find_car(entry_queue, new_car->rego)) || 
            !(find_car(entry_queue, new_car->rego)))
            return new_car;
    }
}


/**
 * Function responsible for managing adding new cars into the system
 * 
 * 1. create new car and add to queue
 * 2. sleep for 1 - 100ms
 */
 
void *car_queuer(void)
{
    for(int i = 0;i < 10; i++)
    {
        // initialise new car and add to queue
        addCar(entry_queue, car_init(entry_queue));
        // sleep for 1 - 100 ms
        usleep(((rand() % 100) + 1) * 1000);    
    }
}