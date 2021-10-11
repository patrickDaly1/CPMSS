#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "linkedlist.h"

#define entrys_exits 5;
#define levels 5;
#define cars_per_level 20;
void *temp_sensor(void);

int main(int argc, char argv)
{    
    /* Task - car generating thread
        1. initialise new thread 
        2. initialise linked list for queue
        2. create new car
            a. each car is given random rego (ie. 123ABC)
            b. each car is given random entry to queue at
        3. add car to the end of the queue
        4. sleep thread for 1 - 100ms
    */

    /* Task - entry checking thread
        1. check which boom gate is ready for new car
        2. locate first car in queue that corresponds to entry
        3. after 2ms pass car rego to manager 
        4. read if car is accepted into 
        5. wait 10ms for boom gate to open 
        6. remove car from queue and add to list of cars in carpark
        7. wait 10ms for boom gate to close 
    */

    /* Task - car driving thread
        1. checks if there are any new cars entering car park
            a. if yes begin thier 10ms trip to parking space (l3 park 15)
            b. once on level trigger lpr on that level
            c. park car for random amount of time
        2. check if any cars are finished parking
            a. if yes begin journey to random exit 10ms 
            b. trigger lpr at exit

        for (;;)
        {
            foreach(car in list)
            {
                if entered
                    begin journey (starts 10ms timer)
                else if driving
                    checks timer < 10ms park car
                else if parked
            }
            update list
        }
    */

   const int THREADS = 1;

   pthread_t threads[THREADS];

   pthread_create(&threads[0], NULL, (void *)temp_sensor, NULL);
   pthread_join(threads[0], NULL);
}

/**
 * Creates new car to add to queue
 * 
 * 1. generate random rego (ie. 123ABC) 
 *  a. ensure this rego is not currently assigned to a car *** FIGURE OUT HOW TO DO THIS ***
 * 2. assign random enterence for car to go to
 */
car_t *car_init(queue_t entry_queue)
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
void car_queuer(queue_t entry_queue)
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
 * Function to simulate temperature sensor
 * 
 * 1. every 1 - 5 seconds update sensor with new temp
 */
void *temp_sensor(void)
{
    for(int i = 0; i < 50; i++)
    {
        // update temp global value here
        // printf("%d\n", (rand() % 4) + 21);
        usleep(((rand() % 5) + 1 ) * 1000);
    }
}

