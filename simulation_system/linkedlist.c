#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "simulator.c"

car_t car_init (void)
{
    // create new car structure
    car_t *new_car = (car_t *)malloc(sizeof(car_t)); 

    // determine random rego
    for (int i = 0; i < 3; i++)
    {
        char* rego[6];
        rego[i] = (char)((rand() % 10));
        rego[i + 3] = (char)((rand() % 26) + 65); 
    }

}
