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
#include "sharedMemory.h"
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

    /* Read in plates.txt and store into linked list */
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
        //free(new_car);
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
        // enterance conditions
        pthread_cond_init(&sharedMem->entrances[i].BG.condition, &cattr);
        pthread_cond_init(&sharedMem->entrances[i].LPR.condition, &cattr);
        pthread_cond_init(&sharedMem->entrances[i].SIGN.condition, &cattr);

        // enterance mutex
        pthread_mutex_init(&sharedMem->entrances[i].BG.lock, &mattr);
        pthread_mutex_init(&sharedMem->entrances[i].SIGN.lock, &mattr);
        pthread_mutex_init(&sharedMem->entrances[i].LPR.lock, &mattr);

        // exit conditions
        pthread_cond_init(&sharedMem->exits[i].BG.condition, &cattr);
        pthread_cond_init(&sharedMem->exits[i].LPR.condition, &cattr);

        // exit mutex
        pthread_mutex_init(&sharedMem->exits[i].BG.lock, &mattr);
        pthread_mutex_init(&sharedMem->exits[i].LPR.lock, &mattr);
    }

    /* Initialise conditions and mutexes for each level */
    for (int i = 0; i < num_levels; i++)
    {
        pthread_mutex_init(&sharedMem->levels[i].LPR.lock, &mattr);
        pthread_cond_init(&sharedMem->levels[i].LPR.condition, &cattr);
    }
    
    for (int i = 0; i < num_levels; ++i) {
        sharedMem->levels[i].tempSen1 = 1;
    }

    sleep(5); // sleep statement to sync systems 

    /* INITIALISE CAR GENERATING THREAD */
    pthread_create(&car_init_thread, NULL, car_queuer, NULL); 

    /* INITIALISE BOOM GATE THREADS */
    int loc[entrys_exits]; // store which entry / exit bg is at
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
    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_cond_destroy(&sharedMem->entrances[i].BG.condition);
        pthread_mutex_destroy(&sharedMem->entrances[i].BG.lock);
        pthread_cond_destroy(&sharedMem->exits[i].BG.condition);
        pthread_mutex_destroy(&sharedMem->exits[i].BG.lock);
    }

    // *** add in destroy statement for level contions

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
 * - this function is responsible for the initialisation of new cars 
 * - registrations are either randomly generated or picked from a predifined list
 * - each car is assigned a random entry to queue behind
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
            // allocates new car and sets rego to one from list
            car_t *car = (car_t *)malloc(sizeof(car_t));
            strcpy(car->rego, plates->data->rego);
            //free(car);

            // takes plate that was just used and moves it to end of list
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

        if (entryQueue == NULL) // check if list is empty
        {
            // signal that new car has been added to specific entry
            pthread_mutex_lock(&(sharedMem->entrances[new_car->entry].BG.lock));
            pthread_cond_signal(&(sharedMem->entrances[new_car->entry].BG.condition));
            pthread_mutex_unlock(&(sharedMem->entrances[new_car->entry].BG.lock));

            return new_car;
        }
        else if (!(search(entryQueue, new_car->rego)) || !(search(inCarpark, new_car->rego)) || 
            !(search(exitQueue, new_car->rego))) // check if car is in system
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
 * Adds new cars to list
 * 
 * - this function is responsible for calling car init and adding the generated car to the queue
 * - the period between cars is randomly determined
 */
 
void *car_queuer(void *arg)
{
    for(int i = 0; i < 400; i++)
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
 * Entry boom gate functions
 * 
 * - this function is responsible for the managing of cars entering the system as well as simulating the functionality of a boom gate
 * - the function find the first car at its corresponding entry
 * - the function will then trigger the lpr and wait for a response from the manager
 * - if denied access the function will remove the car from the simulation
 * - if allowed in the funciton will wait for the manager to signal opening and closing of the boom gates 
 * - while the boom gate is open the function will create a new car thread with all the relevent details 
 * - function will also remove the car from the entry queue and add it to the in carpark linked list
 */
void *boom_gate_entry(void *ptr)
{
    
    int entry = *((int *)ptr); // entry number for boom gate
    car_t* curr_car;

    // loop infinately
    for(;;)
    {
        // create temporary car
        

        // waits to be signaled that car is at entry
        // pthread_mutex_lock(&sharedMem->entrances[entry].BG.lock);
        // pthread_cond_wait(&sharedMem->entrances[entry].BG.condition, &sharedMem->entrances[entry].BG.lock);
        // pthread_mutex_unlock(&sharedMem->entrances[entry].BG.lock);

        usleep(2000); // wait for car

        curr_car =  searchEntry(entryQueue, entry); // search for first car in entry
        
        if ((curr_car != NULL)) // if car has been found
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
            // if denied access
            if ((sharedMem->entrances[entry].SIGN.display == 'X') || (sharedMem->entrances[entry].SIGN.display == 'F') || (&sharedMem->entrances[entry].SIGN.display == NULL))
            {
                /* remove from sim */ 
                pthread_mutex_lock(&lock_queue);
                deleteNode(&entryQueue, curr_car->rego);
                pthread_mutex_unlock(&lock_queue);
            }
            else // if allowed in
            {
                //printf("Boom %d status 1: %c\n", entry, sharedMem->entrances[entry].BG.status);
                /* wait for open boom gate signal */
                pthread_mutex_lock(&(sharedMem->entrances[entry].BG.lock));
                while (sharedMem->entrances[entry].BG.status != 'R')
                    pthread_cond_wait(&sharedMem->entrances[entry].BG.condition, &(sharedMem->entrances[entry].BG.lock));
                pthread_mutex_unlock(&(sharedMem->entrances[entry].BG.lock));
                if(sharedMem->entrances[entry].BG.status != 'R') {
                    printf("Boom gate %d: ", entry);
                    perror("Error raising boom entry\n");
                }
                usleep(10000); // simulat eopening of boom gate
                //printf("Boom %d status 2: %c\n", entry, sharedMem->entrances[entry].BG.status);

                /* signal that boom gate is open */ 
                pthread_mutex_lock(&(sharedMem->entrances[entry].BG.lock));
                //printf("Boom %d status 2: %c\n", entry, sharedMem->entrances[entry].BG.status);
                sharedMem->entrances[entry].BG.status = 'O';
                pthread_cond_signal(&sharedMem->entrances[entry].BG.condition);
                pthread_mutex_unlock(&(sharedMem->entrances[entry].BG.lock));

                //printf("Boom %d status 3: %c\n", entry, sharedMem->entrances[entry].BG.status);

                /* change which linked list car is in */ 
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

                /* wait for closing boom gate */
                pthread_mutex_lock(&(sharedMem->entrances[entry].BG.lock));
                while (sharedMem->entrances[entry].BG.status != 'L')
                    pthread_cond_wait(&sharedMem->entrances[entry].BG.condition, &(sharedMem->entrances[entry].BG.lock));
                pthread_mutex_unlock(&(sharedMem->entrances[entry].BG.lock));
                if(sharedMem->entrances[entry].BG.status != 'L') {
                    printf("Boom gate %d: ", entry);
                    perror("Error lowering boom entry\n");
                }
                
                usleep(10000); // simulate boom gate closing
                //printf("Boom %d status 5: %c\n", entry, sharedMem->entrances[entry].BG.status);

                /* signal that boom gate is closed */                
                pthread_mutex_lock(&(sharedMem->entrances[entry].BG.lock));
                //printf("Boom %d status 2: %c\n", entry, sharedMem->entrances[entry].BG.status);
                sharedMem->entrances[entry].BG.status = 'C';
                pthread_cond_signal(&sharedMem->entrances[entry].BG.condition);
                pthread_mutex_unlock(&(sharedMem->entrances[entry].BG.lock));
                //printf("Boom %d status 6: %c\n", entry, sharedMem->entrances[entry].BG.status);
                
            }
        }  

        //free(curr_car);     
    }

    return NULL;
}

/** 
 * Exit boom gate functions
 * 
 * - this function is responsible for managing cars leaving the carpark     
 * - funtion checks exit queue to find first care at corresponding exit
 * - funtion then triggers lpr and waits for manager to give open signal
 * - function then simulatates opening and closing of boom gates
 * - while boom gates are open function removes car from exit queue
 */
void *boom_gate_exit(void *ptr)
{
    int exit = *((int *)ptr); // exit boom gate is connected to
    car_t* curr_car;

    // loop infinately
    for(;;)
    {
        

        // waits to be signaled that car is at entry
        //pthread_mutex_lock(&sharedMem->exits[exit].BG.lock);
        //pthread_cond_wait(&sharedMem->exits[exit].BG.condition, &sharedMem->entrances[exit].BG.lock);
        //pthread_mutex_unlock(&sharedMem->exits[exit].BG.lock);

        usleep(2000); // wait for car

        // search to find first car at corresponding exit queue
        pthread_mutex_lock(&lock_queue);
        curr_car =  searchExit(exitQueue, exit);;
        pthread_mutex_unlock(&lock_queue);
        
        if (curr_car != NULL) // if car is found
        {
            printf ("rego at exit %d: %s\n", exit, curr_car->rego);
            /* trigger lpr */
            pthread_mutex_lock(&(sharedMem->exits[exit].LPR.lock));
            strcpy(sharedMem->exits[exit].LPR.rego, curr_car->rego);
            pthread_cond_signal(&(sharedMem->exits[exit].LPR.condition));
            pthread_mutex_unlock(&(sharedMem->exits[exit].LPR.lock));
            
            //printf("Boom %d status 1: %c\n", exit, sharedMem->exits[exit].BG.status);

            /* wait for open signal from manager */
            pthread_mutex_lock(&(sharedMem->exits[exit].BG.lock));
            while (sharedMem->exits[exit].BG.status != 'R')
                pthread_cond_wait(&sharedMem->exits[exit].BG.condition, &(sharedMem->exits[exit].BG.lock));
            pthread_mutex_unlock(&(sharedMem->exits[exit].BG.lock));
            if(sharedMem->exits[exit].BG.status != 'R') {
                printf("Boom gate %d: ", exit);
                perror("Error lowering boom exit\n");
            }

            usleep(10000); // simulate boom gate opening

            /* signal that boom gate is open */
            pthread_mutex_lock(&(sharedMem->exits[exit].BG.lock));
            sharedMem->exits[exit].BG.status = 'O';
            pthread_cond_signal(&sharedMem->exits[exit].BG.condition);
            pthread_mutex_unlock(&(sharedMem->exits[exit].BG.lock));
            //printf("Boom %d status 2: %c\n", exit, sharedMem->exits[exit].BG.status);

            /* remove car from exit queue */
            pthread_mutex_lock(&lock_queue);
            deleteNode(&exitQueue, curr_car->rego);
            carsExited++;
            pthread_mutex_unlock(&lock_queue);
            //printf("%d %d %d \n", carsEntered, carsParked, carsExited);
            //printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));
            //printf("%d %d %d %d\n", added, carsEntered, carsParked, carsExited);

            /* wait for manager to lower boom gate */            
            pthread_mutex_lock(&(sharedMem->exits[exit].BG.lock));
            while (sharedMem->exits[exit].BG.status != 'L')
                pthread_cond_wait(&sharedMem->exits[exit].BG.condition, &(sharedMem->exits[exit].BG.lock));
            pthread_mutex_unlock(&(sharedMem->exits[exit].BG.lock));
            if(sharedMem->exits[exit].BG.status != 'L') {
                printf("Boom gate %d: ", exit);
                perror("Error lowering boom exit\n");
            }

            usleep(10000); // simulate boom gate closing  

            /* signal that boom gate is closed */
            pthread_mutex_lock(&(sharedMem->exits[exit].BG.lock));
            sharedMem->exits[exit].BG.status = 'C';
            pthread_cond_signal(&sharedMem->exits[exit].BG.condition);
            pthread_mutex_unlock(&(sharedMem->exits[exit].BG.lock));  

        }
        // if (100 == carsExited)
        // {
        //     break;
        // }

        //free(curr_car);
    }
}

/**
 * Car movement
 * 
 * - this function is resonsible for simulating the movement of the car through the carpark
 */
void *car_movement(void *aCar)
{
    
    car_t* currCar= ((car_t *)aCar);
    printf("Car at level: %d\n", currCar->parking + 1);
    usleep(10000); // simulate traveling to parking
    

    /* trigger level lrp that car is parking on */
    pthread_mutex_lock(&sharedMem->levels[currCar->parking].LPR.lock);
    strcpy(sharedMem->levels[currCar->parking].LPR.rego, currCar->rego);
    pthread_cond_signal(&sharedMem->levels[currCar->parking].LPR.condition);
    pthread_mutex_unlock(&sharedMem->levels[currCar->parking].LPR.lock);

    usleep(((rand() % 900) + 101) * 1000); // park for random amount of time
    //usleep(1000*1000);

    currCar->exit = rand() % entrys_exits;  // select random exit to go towords
    printf("to exit: %d\n", currCar->exit);

    usleep(10000); // simulate traveling to exit

    /* once at exit remove car from in carpark list and add to exit queue */
    pthread_mutex_lock(&lock_queue);
    append(&exitQueue, currCar);
    deleteNode(&inCarpark, currCar->rego);
    carsParked++;
    pthread_mutex_unlock(&lock_queue);
    //printf("entry: %d, carpark: %d, exit: %d\n", listCount(entryQueue), listCount(inCarpark), listCount(exitQueue));
    /* trigger lrp of level car was parked on */
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
 * Temperature simulator
 * 
 * - this function is responsible for updating the temperature on each level every 1 - 5 seconds
 */
void *temp_sensor(void *arg)
{
    int count = 0;
    for(;;)
    {
        if (count < 400)
        {
            // update temp global value here
            //printf("%d from 1\n", (rand() % 4) + 20);
            for(int i = 0; i < num_levels; i++)
                sharedMem->levels[i].tempSen1 = (short)((rand() % 10) + 20);

            usleep(((rand() % 5) + 20) * 1000);
        }
        else
        {
            // update temp global value here
            //printf("%d from 1\n", (rand() % 4) + 58);
            for(int i = 0; i < num_levels; i++)
                sharedMem->levels[i].tempSen1 = (short)((rand() % 10) + 58);

            usleep(((rand() % 5) + 20) * 1000);
        }
        count++;
    }
}
