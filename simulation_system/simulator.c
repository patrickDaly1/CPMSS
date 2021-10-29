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
#include "hashtable.h"

/* Define carpark constraints */
#define entrys_exits 5
#define num_levels 5
#define cars_per_level 20

/* Creat head to linked lists */
struct Node* entryQueue = NULL;
struct Node* inCarpark = NULL;
struct Node* exitQueue = NULL;
struct Node* plates = NULL;

/* Shared memory values */
size_t shmSize = 2920;
int shm_fd;
shm *sharedMem;
const char *key = "PARKING";

/* Other variables */
pthread_mutex_t lock_queue;
char lastRego[6] = "029MZH";
int carsEntered, carsParked, carsExited, added;


/* Initialise functions*/
car_t *car_init(void);
void *car_queuer(void *arg);
void *boom_gate_entry(void *ptr);
void *car_movement(void *aCar);
void *boom_gate_exit(void *ptr);
void *temp_sensor(void *arg);

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

    /* Read in plates.txt */
    //Setup file reader
    FILE *fp;
    size_t len = 10;
    char *line;
    size_t read;
    if((fp = fopen("plates.txt", "r")) == NULL) {
        perror("fopen\n");
        return 1;
    }
    if((line = (char *)malloc(len * sizeof(char))) == NULL) {
        perror("Unable to allocate memory for line\n");
        exit(2);
    }
    //Read plates.txt per line and store in hashtable
    while((read = getline(&line, &len, fp)) != -1) { //function
        char copy[read];
        strncpy(copy, line, read - 1);

        // add to linked list
        car_t *new_car = (car_t *)malloc(sizeof(car_t));
        strcpy(new_car->rego, copy);
        append(&plates, new_car);
    }
    free(line);
    fclose(fp);

    /* Create thread variables */
    pthread_t temp_sensor_thread;
    pthread_t car_init_thread;
    pthread_t boom_gate_entry_thread[entrys_exits];
    pthread_t boom_gate_exit_thread[entrys_exits];

    /* Initialise mattr & cattr */
    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

    /* Initialise conditions and mutexes for each enterence and exit */
    pthread_mutex_init(&lock_queue, NULL);
    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_cond_init(&sharedMem->entrances[i].BG.condition, &cattr);
        pthread_cond_init(&sharedMem->entrances[i].LPR.condition, &cattr);
        pthread_cond_init(&sharedMem->entrances[i].SIGN.condition, &cattr);

        pthread_mutex_init(&sharedMem->entrances[i].BG.lock, &mattr);
        pthread_mutex_init(&sharedMem->entrances[i].SIGN.lock, &mattr);
        pthread_mutex_init(&sharedMem->entrances[i].LPR.lock, &mattr);

        pthread_cond_init(&sharedMem->exits[i].BG.condition, &cattr);
        pthread_cond_init(&sharedMem->exits[i].LPR.condition, &cattr);

        pthread_mutex_init(&sharedMem->exits[i].BG.lock, &mattr);
        pthread_mutex_init(&sharedMem->exits[i].LPR.lock, &mattr);

        

        //initialise levels
    }

    /* Initialise conditions and mutexes for each level */
    for (int i = 0; i < num_levels; i++)
    {
        pthread_mutex_init(&sharedMem->levels[i].LPR.lock, &mattr);
        pthread_cond_init(&sharedMem->levels[i].LPR.condition, &cattr);
    }

    sleep(5); // sleep statement to sync manager

    /* INITIALISE CAR GENERATING THREAD */
    pthread_create(&car_init_thread, NULL, car_queuer, NULL); 

    /* INITIALISE BOOM GATE THREADS */
    int loc[entrys_exits];
    for (int i = 0; i < entrys_exits; i++)
    {
        loc[i] = i;
        pthread_create(&boom_gate_entry_thread[i], NULL, boom_gate_entry, &loc[i]);
    }
    
    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_create(&boom_gate_exit_thread[i], NULL, boom_gate_exit, &loc[i]);
    }

    /* INITIALISE TEMP SENSOR THREADS */
    pthread_create(&temp_sensor_thread, NULL, temp_sensor, NULL);

    /* Join threads */
    pthread_join(car_init_thread, NULL);
    pthread_join(temp_sensor_thread, NULL);
    for (int i = 0; i < entrys_exits; i++)
        pthread_join(boom_gate_entry_thread[i], NULL);

    for (int i = 0; i < entrys_exits; i++) 
        pthread_join(boom_gate_exit_thread[i], NULL);
    

    for(;;){}

    /* CLOSING */

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
car_t *car_init(void)
{
    // create new car structure
    car_t *new_car = (car_t *)malloc(sizeof(car_t));

    // odds for picking rego from plates.txt
    int odds = rand() % 2;

    // create random entry for car
    new_car->entry = rand() % entrys_exits;
    for (;;)
    {
        if((odds == 0 && plates->next != NULL)) // from plates.txt
        {
            car_t *car = (car_t *)malloc(sizeof(car_t));
            strcpy(car->rego, plates->data->rego);

            append(&plates, car);
            deleteNode(&plates, plates->data->rego);
            strcpy(new_car->rego, plates->data->rego);
        }
        else // generate random
        {
            // generate 3 random chars and numbers
            for(int i = 0; i < 3; i++)
            {
                new_car->rego[i + 3] = (char)((rand() % 26) + 65);
                new_car->rego[i] = (rand() % 10) + '0';
            }
        }

        // check if car is already in list
        if (entryQueue == NULL)
        {
            // signal that new car has been added to specific entry
            pthread_mutex_lock(&(sharedMem->entrances[new_car->entry].BG.lock));
            pthread_cond_signal(&(sharedMem->entrances[new_car->entry].BG.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[new_car->entry].BG.lock));

            return new_car;
        }
        else if (!(search(entryQueue, new_car->rego)) || !(search(inCarpark, new_car->rego)) || 
            !(search(exitQueue, new_car->rego)))
        {
            // signal that new car has been added to specific entry
            pthread_mutex_lock(&(sharedMem->entrances[new_car->entry].BG.lock));
            pthread_cond_signal(&(sharedMem->entrances[new_car->entry].BG.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[new_car->entry].BG.lock));

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
 
void *car_queuer(void *arg)
{
    for(int i = 0; i < 1000; i++)
    //for(;;)
    {
        // initialise new car and add to queue
        pthread_mutex_lock(&lock_queue);
        append(&entryQueue, car_init());
        added++;
        pthread_mutex_unlock(&lock_queue);
        //printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));
        
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
    
    int entry = *((int *)ptr); // entry number for boom gate

    // loop infinately
    for(;;)
    {
        car_t* curr_car;

        // waits to be signaled that car is at entry
        // pthread_mutex_lock(&sharedMem->entrances[entry].BG.lock);
        // pthread_cond_wait(&sharedMem->entrances[entry].BG.condition, &sharedMem->entrances[entry].BG.lock);
        // pthread_mutex_unlock(&sharedMem->entrances[entry].BG.lock);

        usleep(2000); // wait for car

        curr_car =  searchEntry(entryQueue, entry);
        
        if ((curr_car != NULL))
        {
            /* Pass rego to LPR */
            pthread_mutex_lock(&(sharedMem->entrances[entry].LPR.lock));
            strcpy(sharedMem->entrances[entry].LPR.rego, curr_car->rego);
            pthread_cond_signal(&(sharedMem->entrances[entry].LPR.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[entry].LPR.lock));

            /* Wait wait for manager to update sign */
            pthread_mutex_lock(&(sharedMem->entrances[entry].SIGN.lock));
            pthread_cond_wait(&(sharedMem->entrances[entry].SIGN.condition), &(sharedMem->entrances[entry].SIGN.lock));
            pthread_mutex_unlock(&(sharedMem->entrances[entry].SIGN.lock));
            
            printf ("rego at entry %d: %s\n", entry, curr_car->rego);
            printf("sign display entry %d: %c\n", entry, sharedMem->entrances[entry].SIGN.display);
            if ((sharedMem->entrances[entry].SIGN.display == 'X') || (sharedMem->entrances[entry].SIGN.display == 'F') || (&sharedMem->entrances[entry].SIGN.display == NULL))
            {
                pthread_mutex_lock(&lock_queue);
                deleteNode(&entryQueue, curr_car->rego);
                pthread_mutex_unlock(&lock_queue);
            }
            else
            {
                //printf("Boom %d status 1: %c\n", entry, sharedMem->entrances[entry].BG.status);
                pthread_mutex_lock(&(sharedMem->entrances[entry].BG.lock));
                while (sharedMem->entrances[entry].BG.status != 'R')
                    pthread_cond_wait(&sharedMem->entrances[entry].BG.condition, &(sharedMem->entrances[entry].BG.lock));
                pthread_mutex_unlock(&(sharedMem->entrances[entry].BG.lock));
                if(sharedMem->entrances[entry].BG.status != 'R') {
                    printf("Boom gate %d: ", entry);
                    perror("Error raising boom entry\n");
                }
                usleep(10000); // open boom gate
                //printf("Boom %d status 2: %c\n", entry, sharedMem->entrances[entry].BG.status);

                pthread_mutex_lock(&(sharedMem->entrances[entry].BG.lock));
                printf("Boom %d status 2: %c\n", entry, sharedMem->entrances[entry].BG.status);
                sharedMem->entrances[entry].BG.status = 'O';
                pthread_cond_signal(&sharedMem->entrances[entry].BG.condition);
                pthread_mutex_unlock(&(sharedMem->entrances[entry].BG.lock));

                //printf("Boom %d status 3: %c\n", entry, sharedMem->entrances[entry].BG.status);

                pthread_mutex_lock(&lock_queue);
                curr_car->parking = (int)(sharedMem->entrances[entry].SIGN.display) - 49;
                append(&inCarpark, curr_car);
                deleteNode(&entryQueue, curr_car->rego);
                carsEntered++;
                pthread_mutex_unlock(&lock_queue);
                //printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));
                /* CREATE NEW CAR THREAD */
                pthread_t car;
                void * arg = malloc(sizeof(car_t));
                arg = curr_car;
                pthread_create(&car, NULL, car_movement, arg); 

                //printf("Boom %d status 4: %c\n", entry, sharedMem->entrances[entry].BG.status);

                pthread_mutex_lock(&(sharedMem->entrances[entry].BG.lock));
                while (sharedMem->entrances[entry].BG.status != 'L')
                pthread_cond_wait(&sharedMem->entrances[entry].BG.condition, &(sharedMem->entrances[entry].BG.lock));
                pthread_mutex_unlock(&(sharedMem->entrances[entry].BG.lock));
                if(sharedMem->entrances[entry].BG.status != 'L') {
                    printf("Boom gate %d: ", entry);
                    perror("Error lowering boom entry\n");
                }
                
                usleep(10000); // close boom gate
                //printf("Boom %d status 5: %c\n", entry, sharedMem->entrances[entry].BG.status);

                pthread_mutex_lock(&(sharedMem->entrances[entry].BG.lock));
                printf("Boom %d status 2: %c\n", entry, sharedMem->entrances[entry].BG.status);
                sharedMem->entrances[entry].BG.status = 'C';
                pthread_cond_signal(&sharedMem->entrances[entry].BG.condition);
                pthread_mutex_unlock(&(sharedMem->entrances[entry].BG.lock));
                //printf("Boom %d status 6: %c\n", entry, sharedMem->entrances[entry].BG.status);
                
            }
        }
        // if ((entryQueue == NULL) && (inCarpark == NULL))
        // { 
        //     break;
        // }
        
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

    // loop infinately
    for(;;)
    {
        car_t* curr_car;

        // waits to be signaled that car is at entry
        //pthread_mutex_lock(&sharedMem->exits[exit].BG.lock);
        //pthread_cond_wait(&sharedMem->exits[exit].BG.condition, &sharedMem->entrances[exit].BG.lock);
        //pthread_mutex_unlock(&sharedMem->exits[exit].BG.lock);

        usleep(2000); // wait for car+
        pthread_mutex_lock(&lock_queue);
        curr_car =  searchExit(exitQueue, exit);;
        pthread_mutex_unlock(&lock_queue);
        
        if (curr_car != NULL)
        {
            // *** PASS REGO TO CORRECT LPR
            pthread_mutex_lock(&(sharedMem->exits[exit].LPR.lock));
            strcpy(sharedMem->exits[exit].LPR.rego, curr_car->rego);
            pthread_cond_signal(&(sharedMem->exits[exit].LPR.condition));
            pthread_mutex_unlock(&(sharedMem->exits[exit].LPR.lock));
            
            //printf("Boom %d status 1: %c\n", exit, sharedMem->exits[exit].BG.status);
            pthread_mutex_lock(&(sharedMem->exits[exit].BG.lock));
            pthread_cond_wait(&sharedMem->exits[exit].BG.condition, &(sharedMem->exits[exit].BG.lock));
            pthread_mutex_unlock(&(sharedMem->exits[exit].BG.lock));
            if(sharedMem->exits[exit].BG.status != 'R') {
                printf("Boom gate %d: ", exit);
                perror("Error lowering boom exit\n");
            }

            usleep(10000); // open boom gate

            pthread_mutex_lock(&(sharedMem->exits[exit].BG.lock));
            sharedMem->exits[exit].BG.status = 'O';
            pthread_cond_signal(&sharedMem->exits[exit].BG.condition);
            pthread_mutex_unlock(&(sharedMem->exits[exit].BG.lock));
            //printf("Boom %d status 2: %c\n", exit, sharedMem->exits[exit].BG.status);

            pthread_mutex_lock(&lock_queue);
            deleteNode(&exitQueue, curr_car->rego);
            carsExited++;
            pthread_mutex_unlock(&lock_queue);
            //printf("%d %d %d \n", carsEntered, carsParked, carsExited);
            //printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));
            //printf("%d %d %d %d\n", added, carsEntered, carsParked, carsExited);
            
            pthread_mutex_lock(&(sharedMem->exits[exit].BG.lock));
            pthread_cond_wait(&sharedMem->exits[exit].BG.condition, &(sharedMem->exits[exit].BG.lock));
            pthread_mutex_unlock(&(sharedMem->exits[exit].BG.lock));
            if(sharedMem->exits[exit].BG.status != 'L') {
                printf("Boom gate %d: ", exit);
                perror("Error lowering boom exit\n");
            }

            usleep(10000); // close boom gate   

            pthread_mutex_lock(&(sharedMem->exits[exit].BG.lock));
            sharedMem->exits[exit].BG.status = 'C';
            pthread_cond_signal(&sharedMem->exits[exit].BG.condition);
            pthread_mutex_unlock(&(sharedMem->exits[exit].BG.lock));  

        }
        // if (100 == carsExited)
        // {
        //     break;
        // }
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
    printf("Car at level: %d\n", currCar->parking + 1);
    usleep(10000); // travel to parking

    pthread_mutex_lock(&sharedMem->levels[currCar->parking].LPR.lock);
    strcpy(sharedMem->levels[currCar->parking].LPR.rego, currCar->rego);
    pthread_cond_signal(&sharedMem->levels[currCar->parking].LPR.condition);
    pthread_mutex_unlock(&sharedMem->levels[currCar->parking].LPR.lock);

    //usleep(((rand() % 900) + 101) * 1000);
    usleep(1000*1000);

    currCar->exit = rand() % entrys_exits;

    usleep(10000); // travel to exit

    pthread_mutex_lock(&lock_queue);
    append(&exitQueue, currCar);
    deleteNode(&inCarpark, currCar->rego);
    carsParked++;
    pthread_mutex_unlock(&lock_queue);
    //printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));

    pthread_mutex_lock(&sharedMem->levels[currCar->parking].LPR.lock);
    strcpy(sharedMem->levels[currCar->parking].LPR.rego, currCar->rego);
    pthread_cond_signal(&sharedMem->levels[currCar->parking].LPR.condition);
    pthread_mutex_unlock(&sharedMem->levels[currCar->parking].LPR.lock);

    // pthread_mutex_lock(&sharedMem->exits[currCar->exit].BG.lock);
    // pthread_cond_signal(&sharedMem->exits[currCar->exit].BG.condition);
    // pthread_mutex_unlock(&sharedMem->exits[currCar->exit].BG.lock);
    

    return 0;
}

/**
 * Function to simulate temperature sensor
 * 
 * 1. every 1 - 5 seconds update sensor with new temp
 */
void *temp_sensor(void *arg)
{
    for(;;)
    {
        // update temp global value here
        //printf("%d from 1\n", (rand() % 4) + 21);
        for(int i = 0; i < num_levels; i++)
            sharedMem->num_levels[i]->tempSens1 = (short)((rand() % 4) + 21);

        usleep(((rand() % 5) + 20) * 1000);
    }
}