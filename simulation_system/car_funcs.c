#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct car_data
{
    char* licence_plate;
    int entry_num;
    char parking_num;  
} car_data_t;
