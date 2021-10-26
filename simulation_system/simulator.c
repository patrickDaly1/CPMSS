#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include "linkedlist.h"
#include "../shared_memory/sharedMemory.h"

#define entrys_exits 5
#define levels 5
#define cars_per_level 20


struct Node* entryQueue = NULL;
struct Node* inCarpark = NULL;
struct Node* exitQueue = NULL;

pthread_mutex_t lock;
pthread_mutex_t lock_queue;
size_t shmSize = 2920;
pthread_cond_t boom_gate_entry_signal[entrys_exits];
pthread_mutex_t boom_gate_entry_lock[entrys_exits];
pthread_cond_t boom_gate_exit_signal[entrys_exits];
pthread_mutex_t boom_gate_exit_lock[entrys_exits];

int boom_gate_entry_signaled[entrys_exits];
int boom_gate_exit_signaled[entrys_exits];
int shm_fd;
shm *sharedMem;
const char *key = "PARKING";

int carsEntered, carsParked, carsExited, added;

car_t *car_init(void);
void *car_queuer(void);
void *boom_gate_entry(void *ptr);
void *car_movement(void *aCar);
void *boom_gate_exit(void *ptr);
void *temp_sensor(void);

int main(int argc, char** argv)
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

    // set threads
    pthread_t temp_sensor_thread;
    pthread_t car_init_thread;
    pthread_t boom_gate_entry_thread[entrys_exits];
    pthread_t boom_gate_exit_thread[entrys_exits];

    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

    pthread_mutex_init(&lock_queue, NULL);
    for (int i = 0; i < entrys_exits; i++)
    {

        pthread_cond_init(&sharedMem->entrances[i].BG.condition, &cattr);
        pthread_mutex_init(&sharedMem->entrances[i].BG.lock, &mattr);
        pthread_cond_init(&sharedMem->exits[i].BG.condition, &cattr);
        pthread_mutex_init(&sharedMem->exits[i].BG.lock, &mattr);
    }

    /* INITIALISE CAR GENERATING THREAD */
    // pthread_create(&car_init_thread, NULL, (void*) car_queuer, NULL); 

    // /* INITIALISE BOOM GATE THREADS */
    // int loc[entrys_exits];
    // for (int i = 0; i < entrys_exits; i++)
    // {
    //     loc[i] = i;
    //     pthread_create(&boom_gate_entry_thread[i], NULL, (void*) boom_gate_entry, &loc[i]);
    // }
    
    // for (int i = 0; i < entrys_exits; i++)
    // {
    //     pthread_create(&boom_gate_exit_thread[i], NULL, (void*) boom_gate_exit, &loc[i]);
    // }

    /* INITIALISE TEMP SENSOR THREADS */
    //pthread_create(&temp_sensor_thread, NULL, (void *) temp_sensor, NULL);

    //close threads first
    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_cond_destroy(&sharedMem->entrances[i].BG.condition);
        pthread_mutex_destroy(&sharedMem->entrances[i].BG.lock);
        pthread_cond_destroy(&sharedMem->exits[i].BG.condition);
        pthread_mutex_destroy(&sharedMem->exits[i].BG.lock);
    }

    /* CLOSE SHARED MEMORY */
    if (munmap(sharedMem, shmSize) != 0) {
        perror("munmap");
    }

    if (shm_unlink(key) != 0) {
        perror("shm_unlink");
    }

    pthread_mutex_destroy(&lock_queue);
    
    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);
}

/**
 * Creates new car to add to queue
 * 
 * 1. generate random rego (ie. 123ABC) 
 *  a. ensure this rego is not currently assigned to a car *** FIGURE OUT HOW TO DO THIS ***
 * 2. assign random enterence for car to go to
 */
// car_t *car_init(void)
// {
//     // create new car structure
//     car_t *new_car = (car_t *)malloc(sizeof(car_t));

//     // generate random rego values
//     int odds = rand() % 4;

//     // create random probability of guarenteed entry
//     new_car->entry = rand() % entrys_exits;
//     //printf("%d\n", new_car->entry);
//     for (;;)
//     {
//         if(odds == -1)
//         {
//             // choose from plates.txt file
//         }
//         else
//         {
//             for(int i = 0; i < 3; i++)
//             {
//                 new_car->rego[i] = (char)((rand() % 26) + 65);
//                 new_car->rego[i + 3] = (rand() % 10) + '0';
//             }
//         }

//         // check if car is already in list
//         if (entryQueue == NULL)
//         {
//             // signal that new car has been added to specific entry
//             boom_gate_entry_signaled[new_car->entry] = true;;
//             pthread_mutex_lock(&boom_gate_entry_lock[new_car->entry]);
//             pthread_cond_signal(&boom_gate_entry_signal[new_car->entry]);
//             pthread_mutex_unlock(&boom_gate_entry_lock[new_car->entry]);

//             return new_car;
//         }

//         else if (!(search(entryQueue, new_car->rego)) || !(search(inCarpark, new_car->rego)) || 
//             !(search(exitQueue, new_car->rego)))
//         {
//             // signal that new car has been added to specific entry
//             boom_gate_entry_signaled[new_car->entry] = true;
//             pthread_mutex_lock(&boom_gate_entry_lock[new_car->entry]);
//             pthread_cond_signal(&boom_gate_entry_signal[new_car->entry]);
//             pthread_mutex_unlock(&boom_gate_entry_lock[new_car->entry]);

//             return new_car;
//         }
//     }
// }


// /**
//  * Function responsible for managing adding new cars into the system
//  * 
//  * 1. create new car and add to queue
//  * 2. sleep for 1 - 100ms
//  */
 
// void *car_queuer(void)
// {
//     for(int i = 0; i < 100; i++)
//     {
//         // initialise new car and add to queue
//         pthread_mutex_lock(&lock_entry_queue);
//         append(&entryQueue, car_init());
//         added++;
//         pthread_mutex_unlock(&lock_entry_queue);
//         printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));
//         // sleep for 1 - 100 ms
//         usleep(((rand() % 100) + 1) * 1000);    
//     }

//     return NULL;
// }

// /** 
//  * Function responsible for managing the entry of new cars
//  * 
//  * 1. sleep thread for (2ms) 
//  * 2. find first car from queue that corresponds to correct enterence. 
//  * 3. read rego from car and update lpr for corresponding enterence and wait for manager response
//  *  a. if denied remove from queue
//  *  b. if allowed proceed with below
//  * 4. sleep for 10ms (boom gate opening)
//  * 5. create new car thread and remove from queue
//  * 6. sleep 10ms (boom gate closing)
//  * 7. loop to top        
//  */
// void *boom_gate_entry(void *ptr)
// {
    
//     int entry = *((int *)ptr);
//     //int entry = 2;
//     //printf("%d @ 2\n", entry);

//     // loop infinately
//     for(;;)
//     {
//         car_t* curr_car;

//         // waits to be signaled that car is at entry
//         pthread_mutex_lock(&boom_gate_entry_lock[entry]);
//         while(!boom_gate_entry_signaled)
//             pthread_cond_wait(&boom_gate_entry_signal[entry], &boom_gate_entry_lock[entry]);
//         pthread_mutex_unlock(&boom_gate_entry_lock[entry]);

//         usleep(2000); // wait for car

//         curr_car =  searchEntry(entryQueue, entry);
        
//         if ((curr_car != NULL))
//         {
//             // *** PASS REGO TO CORRECT LPR
            
//             if (false)
//             {
//                 deleteNode(&entryQueue, curr_car->rego);
//             }
//             else if (true)
//             {
//                 usleep(10000); // open boom gate
                
//                 pthread_mutex_lock(&lock_entry_queue);
//                 append(&inCarpark, curr_car);
//                 deleteNode(&entryQueue, curr_car->rego);
//                 carsEntered++;
//                 pthread_mutex_unlock(&lock_entry_queue);
//                 printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));

//                 /* CREATE NEW CAR THREAD */
//                 pthread_t car;
//                 void * arg = malloc(sizeof(car_t));
//                 arg = curr_car;
//                 pthread_create(&car, NULL, (void*) car_movement, arg); 
                
//                 usleep(10000); // close boom gate
                
//             }
//         }
//         if ((entryQueue == NULL) && (inCarpark == NULL))
//         { 
//             break;
//         }
        
//     }
//     return NULL;
// }

// /** 
//  * Function responsible for managing the entry of new cars
//  * 
//  * 1. sleep thread for (2ms) 
//  * 2. find first car from queue that corresponds to correct exit. 
//  * 3. read rego from car and update lpr for corresponding enterence and wait for manager response
//  * 4. sleep for 10ms (boom gate opening)
//  * 5. create new car thread and remove from queue
//  * 6. sleep 10ms (boom gate closing)
//  * 7. loop to top        
//  */
// void *boom_gate_exit(void *ptr)
// {
//     int exit = *((int *)ptr);
//     bool run_once = false;

//     // loop infinately
//     for(;;)
//     {
//         car_t* curr_car;

//         // waits to be signaled that car is at entry
//         pthread_mutex_lock(&boom_gate_exit_lock[exit]);
//         while(!boom_gate_exit_signaled)
//             pthread_cond_wait(&boom_gate_exit_signal[exit], &boom_gate_exit_lock[exit]);
//         pthread_mutex_unlock(&boom_gate_exit_lock[exit]);

//         usleep(2000); // wait for car+
//         pthread_mutex_lock(&lock_entry_queue);
//         curr_car =  searchExit(exitQueue, exit);;
//         pthread_mutex_unlock(&lock_entry_queue);
        
//         if (curr_car != NULL)
//         {
//             // *** PASS REGO TO CORRECT LPR
//             usleep(10000); // open boom gate
            
            
//             pthread_mutex_lock(&lock_entry_queue);
//             deleteNode(&exitQueue, curr_car->rego);
//             carsExited++;
//             pthread_mutex_unlock(&lock_entry_queue);
//             //printf("%d %d %d \n", carsEntered, carsParked, carsExited);
//             printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));
            
//             usleep(10000); // close boom gate     

//         }
//         if (100 == carsExited)
//         {
//             break;
//         }
//     }
// }

// /**
//  * Function to manage the simulation of car within carpark
//  * 
//  * 1. sleep for 10ms (traveling to parking)
//  * 2. trigger LPR for corresponding level
//  * 3. sleep for 100 - 1000 ms
//  * 4. trigger level lpr on exit
//  * 5. sleep for 10ms (traveling to random exit)
//  * 6. trigger corresponding exit LRP
//  */
// void *car_movement(void *aCar)
// {
    
//     car_t* currCar= ((car_t *)aCar);
//     usleep(10000); // travel to parking

//     usleep(((rand() % 900) + 101) * 1000);

//     currCar->exit = rand() % entrys_exits;

//     usleep(10000); // travel to exit

//     pthread_mutex_lock(&lock_entry_queue);
//     append(&exitQueue, currCar);
//     deleteNode(&inCarpark, currCar->rego);
//     carsParked++;
//     pthread_mutex_unlock(&lock_entry_queue);
//     printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));
    
//     boom_gate_exit_signaled[currCar->exit] = true;
//     pthread_mutex_lock(&boom_gate_exit_lock[currCar->exit]);
//     pthread_cond_signal(&boom_gate_exit_signal[currCar->exit]);
//     pthread_mutex_unlock(&boom_gate_exit_lock[currCar->exit]);
    

//     return 0;
// }

// /**
//  * Function to simulate temperature sensor
//  * 
//  * 1. every 1 - 5 seconds update sensor with new temp
//  */
// void *temp_sensor(void)
// {
//     for(;;)
//     {
//         // update temp global value here
//         //printf("%d from 1\n", (rand() % 4) + 21);
//         for(int i = 0; i < level; i++)
//             sharedMem->levels[i]->tempSens1 = (short)((rand() % 4) + 21);

//         usleep(((rand() % 5) + 20) * 1000);
//     }
// }