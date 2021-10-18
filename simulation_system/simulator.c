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


queue_t *entry_queue;
queue_t *incarpark_queue;
queue_t *exit_queue;
pthread_mutex_t lock;
pthread_mutex_t lock_queue;
size_t shmSize = 2920;

int shm_fd;
shm *sharedMem;
const char *key = "PARKING";

car_t *car_init(void);
void *car_queuer(void);
void *boom_gate_entry(void *ptr);
void *car_movement(void *aCar);
void *boom_gate_exit(void *ptr);
void *temp_sensor(void);

int main(int argc, char argv)
{ 
    /* OPEN SHARED MEMORY */
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

    entry_queue = createQueue();
    incarpark_queue = createQueue();
    exit_queue = createQueue();

    // set threads
    pthread_t temp_sensor_thread;
    pthread_t car_init_thread;
    pthread_t boom_gate_entry_thread[entrys_exits];
    pthread_t boom_gate_exit_thread[entrys_exits];

    pthread_mutex_init(&lock_queue, NULL);
    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_cond_init(&boom_gate_entry_signal[i], NULL);
        pthread_mutex_init(&boom_gate_entry_lock[i], NULL);
        pthread_cond_init(&boom_gate_exit_signal[i], NULL);
        pthread_mutex_init(&boom_gate_exit_lock[i], NULL);

        boom_gate_entry_signaled[i] = false;
        boom_gate_exit_signaled[i] = false;
    }

    /* INITIALISE CAR GENERATING THREAD */
    pthread_create(&car_init_thread, NULL, (void*) car_queuer, NULL); 

    /* INITIALISE BOOM GATE THREADS */
    int loc[entrys_exits];
    for (int i = 0; i < entrys_exits; i++)
    {
        loc[i] = i;
        pthread_create(&boom_gate_entry_thread[i], NULL, (void*) boom_gate_entry, &loc[i]);
    }

    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_create(&boom_gate_exit_thread[i], NULL, (void*) boom_gate_exit, &loc[i]);
    }

    /* INITIALISE TEMP SENSOR THREADS */
    pthread_create(&temp_sensor_thread, NULL, (void *) temp_sensor, NULL);

    /* wait for thread to finish exicuting */
    pthread_join(temp_sensor_thread, NULL);
    pthread_join(car_init_thread, NULL);
    for (int i = 0; i < entrys_exits; i++)
        pthread_join(boom_gate_entry_thread[i], NULL);

    for (int i = 0; i < entrys_exits; i++)
        pthread_join(boom_gate_exit_thread[i], NULL);

    /* CLOSE SHARED MEMORY */
    if (munmap(sharedMem, shmSize) != 0) {
        perror("munmap");
    }

    if (shm_unlink(key) != 0) {
        perror("shm_unlink");
    }

    pthread_mutex_destroy(&lock_queue);
    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_cond_destroy(&boom_gate_entry_signal[i]);
        pthread_mutex_destroy(&boom_gate_entry_lock[i]);
        pthread_cond_destroy(&boom_gate_exit_signal[i]);
        pthread_mutex_destroy(&boom_gate_exit_lock[i]);
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
    //printf("%d\n", new_car->entry);
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
            // signal that new car has been added to specific entry
            boom_gate_entry_signaled[new_car->entry] = true;
            pthread_mutex_lock(&boom_gate_entry_lock[new_car->entry]);
            pthread_cond_signal(&boom_gate_entry_signal[new_car->entry]);
            pthread_mutex_unlock(&boom_gate_entry_lock[new_car->entry]);

            return new_car;

        else if (!(findCarRego(entry_queue, new_car->rego)) || !(findCarRego(entry_queue, new_car->rego)) || 
            !(findCarRego(entry_queue, new_car->rego)))
        {
            // signal that new car has been added to specific entry
            boom_gate_entry_signaled[new_car->entry] = true;
            pthread_mutex_lock(&boom_gate_entry_lock[new_car->entry]);
            pthread_cond_signal(&boom_gate_entry_signal[new_car->entry]);
            pthread_mutex_unlock(&boom_gate_entry_lock[new_car->entry]);

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
 
void *car_queuer(void)
{
    for(int i = 0; i < 30; i++)
    {
        // initialise new car and add to queue
        pthread_mutex_lock(&lock_queue)
        addCar(entry_queue, car_init());
        pthread_mutex_unlock(&lock_queue);
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
void *boom_gate_entry(void *ptr)
{
    
    int entry = *((int *)ptr);
    //int entry = 2;
    //printf("%d @ 2\n", entry);

    // loop infinately
    for(;;)
    {
        car_t* curr_car;

        usleep(2000); // wait for car

        curr_car =  findFirstCarEntry(entry_queue, entry);
        
        if ((curr_car != NULL) && (entry_queue->front != NULL))
        {
            // *** PASS REGO TO CORRECT LPR
            
            if (false)
            {
                entry_queue->front = removeCarRego(entry_queue->front, curr_car);
            }
            else if (true)
            {
                usleep(10000); // open boom gate
                pthread_mutex_lock(sharedMem->entrances[entry]->BG->lock);
                sharedMem->entrances[entry]->BG->status = "o";
                pthread_mutex_unlock(sharedMem->entrances[entry]->BG->lock);
                
                pthread_mutex_lock(&lock_queue);
                addCar(incarpark_queue, curr_car);
                entry_queue->front = removeCarRego(entry_queue->front, curr_car);
                pthread_mutex_unlock(&lock_queue);

                pthread_mutex_lock(sharedMem->entrances[entry]->LPR->lock);
                stpcpy(sharedMem->entrances[entry]->LPR->rego, curr_car->rego);
                pthread_mutex_unlock(sharedMem->entrances[entry]->LPR->lock);

                /* CREATE NEW CAR THREAD */
                pthread_t car;
                void * arg = malloc(sizeof(car_t));
                arg = curr_car;
                pthread_create(&car, NULL, (void*) car_movement, arg); 
                
                usleep(10000); // close boom gate

                pthread_mutex_lock(sharedMem->entrances[entry]->BG->lock);
                sharedMem->entrances[entry]->BG->status = "c";
                pthread_mutex_unlock(sharedMem->entrances[entry]->BG->lock);               
            }
        }
        else // if no cars left in entry queue
        { 
            break;
        }
        
    }
    return NULL;
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
void *boom_gate_exit(void *ptr)
{
    int exit = *((int *)ptr);
    bool run_once = false;

    // loop infinately
    for(;;)
    {
        car_t* curr_car;

        usleep(2000); // wait for car+
        curr_car =  findFirstCarEntry(exit_queue, exit);

        
        if ((curr_car != NULL) && exit_queue->front != NULL)
        {
            // *** PASS REGO TO CORRECT LPR
            usleep(10000); // open boom gate
            pthread_mutex_lock(sharedMem->exits[exit]->BG->lock);
            sharedMem->exits[exit]->BG->status = "o";
            pthread_mutex_unlock(sharedMem->exits[exit]->BG->lock);
            
            pthread_mutex_lock(&lock_queue);
            exit_queue->front = removeCarRego(exit_queue->front, curr_car);
            pthread_mutex_unlock(&lock_queue);

            pthread_mutex_lock(sharedMem->exits[exit]->LPR->lock);
            stpcpy(sharedMem->exits[exit]->LPR->rego, curr_car->rego);
            pthread_mutex_unlock(sharedMem->exits[exit]->LPR->lock);
            
            usleep(10000); // close boom gate    
            pthread_mutex_lock(sharedMem->exits[exit]->BG->lock);
            sharedMem->exits[exit]->BG->status = "c";
            pthread_mutex_unlock(sharedMem->exits[exit]->BG->lock);  
        }
        else
        {
            break;
        }
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
void *car_movement(void *aCar)
{
    
    car_t* currCar= ((car_t *)aCar);
    usleep(10000); // travel to parking

    usleep(((rand() % 900) + 101) * 1000);

    currCar->exit = rand() % entrys_exits;

    usleep(10000); // travel to exit

    pthread_mutex_lock(&lock_queue);
    addCar(exit_queue, currCar);
    incarpark_queue->front = removeCarRego(incarpark_queue->front, currCar);
    pthread_mutex_unlock(&lock_queue);

    return 0;
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
        //printf("%d from 1\n", (rand() % 4) + 21);
        for(int i = 0; i < level; i++)
            sharedMem->levels[i]->tempSens1 = (short)((rand() % 4) + 21);

        usleep(((rand() % 5) + 20) * 1000);
    }
}