#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "linkedlist.h"
#include "../shared_memory/sharedMemory.h"

#define entrys_exits 5;
#define levels 5;
#define cars_per_level 20;

pthread_mutex_t lock;
queue_t *entry_queue = createQueue();
queue_t *incarpark_queue = createQueue();
queue_t *exit_queue = createQueue();

void *temp_sensor(void);
car_t *car_init(void);
void *car_queuer(void);

int main(int argc, char argv)
{ 
    /* OPEN SHARED MEMORY */
    size_t shmSize = 2920;
    int shm_fd;
    shm *sharedMem;
    const char *key = "PARKING";

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

    // set threads
    pthread_t temp_sensor_thread, car_init_thread;
    pthread_t boom_gate_entry_thread[entrys_exits], boom_gate_exit_thread[entrys_exits];

    /* INITIALISE CAR GENERATING THREAD */
    pthread_create(&car_init_thread, NULL, (void *) car_init, entry_queue); 

    /* INITIALISE BOOM GATE THREADS */
    
    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_create(&boom_gate_entry_thread[i], NULL, (void *) boom_gate_entry, i);
    }

    /* INITIALISE TEMP SENSOR THREADS */
    pthread_create(&temp_sensor_thread, NULL, (void *) temp_sensor, NULL);

    /* wait for thread to finish exicuting */
    pthread_join(temp_sensor_thread, NULL);
    pthread_join(car_init, NULL);
    for (int i = 0; i < entrys_exits*2, i++)
    {
        pthread_join(boom_gate_entry_thread[i]);
    }

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
                new_car->rego[i + 3] = (rand() % 10) + '0';
            }
        }

        // check if car is already in list
        if (entry_queue->front == NULL)
            return new_car;

        else if (!(findCarRego(entry_queue, new_car->rego)) || !(findCarRego(entry_queue, new_car->rego)) || 
            !(findCarRego(entry_queue, new_car->rego)))
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
    for(int i = 0; i < 50; i++)
    {
        // initialise new car and add to queue
        addCar(entry_queue, car_init());
        // sleep for 1 - 100 ms
        usleep(((rand() % 100) + 1) * 1000);    
    }

    return NULL;
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
void *boom_gate_entry(int entry)
{
    // loop infinately
    for(;;)
    {
        car_t* curr_car;

        usleep(2000); // wait for car

        curr_car =  findFirstCarEntry(data->entry_queue, data->entry);

        // *** PASS REGO TO CORRECT LPR

        if (/*rejected*/)
        {
            removeCar(data->entry_queue, curr_car->rego);
        }
        else if (/*accepted*/)
        {
            usleep(10000); // open boom gate

            addCar(data->incarpark_queue, curr_car);
            removeCar(data->entry_queue, curr_car->rego);

            /* CREATE NEW CAR THREAD */
            pthread_t car;
            pthread_create(car, NULL, car_movement, curr_car); 

            usleep(10000); // close boom gate
        }
    }
}

/** 
 * Function responsible for managing the entry of new cars
 * 
 * 1. sleep thread for (2ms) 
 * 2. find first car from queue that corresponds to correct exit. 
 * 3. read rego from car and update lpr for corresponding enterence and wait for manager response
 * 4. sleep for 10ms (boom gate opening)
 * 5. create new car thread and remove from queue
 * 6. sleep 10ms (boom gate closing)
 * 7. loop to top        
 */
void *boom_gate_exit(int exit)
{
    // loop infinately
    for(;;)
    {
        car_t* curr_car;

        usleep(2000); // wait for car

        curr_car =  *findFirstCarExit(entry_queue, exit); // *** add for if car isnt found ***

        // *** PASS REGO TO CORRECT LPR

        removeCar(exit_queue, curr_car->rego);
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
void *car_movement(car_t *aCar)
{
    usleep(10000); // travel to parking

    usleep(((rand() % 900) + 101) * 1000);

    aCar->exit = rand() % entrys_exits;

    usleep(10000); // travel to exit

    addCar(exit_queue, addCar);
    removeCar(incarpark_queue, aCar->rego);
}

/**
 * Function to simulate temperature sensor
 * 
 * 1. every 1 - 5 seconds update sensor with new temp
 */
void *temp_sensor(void)
{
    for(;;)
    {
        // update temp global value here
        printf("%d from 1\n", (rand() % 4) + 21);
        usleep(((rand() % 5) + 20) * 1000);
    }
}