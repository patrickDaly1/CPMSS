#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "linkedlist.h"

#define entrys_exits 5;
#define levels 5;
#define cars_per_level 20;

main()
{    
    /* Task - car generating thread
        1. initialise new thread 
        2. initialise linke list for queue
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
        create new thread 
        [1223, 1234,1234,1234]
    */
}

/**
 * Creates new car to add to queue
 * 
 * 1. generate random rego (ie. 123ABC)
 * 2. assign random enterence for car to go to
 */
car_t car_init(void)
{
    car_t *new_car = (car_t *)malloc(sizeof(car_t));

    for(int i = 0; i < 3; i++)
    {
        new_car->rego[i];
    }
}

