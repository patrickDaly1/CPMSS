#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "linkedlist.h"

#define entrys_exits 5
#define levels 5
#define cars_per_level 20

car_t *car_init(void);
void *car_queuer(void);
void *boom_gate_entry(void *ptr);
void *car_movement(void *aCar);
void *boom_gate_exit(void *ptr);

queue_t *entry_queue;
queue_t *incarpark_queue;
queue_t *exit_queue;

pthread_mutex_t lock_queue;

pthread_cond_t boom_gate_entry_signal[entrys_exits];
pthread_mutex_t boom_gate_entry_lock[entrys_exits];
pthread_cond_t boom_gate_exit_signal[entrys_exits];
pthread_mutex_t boom_gate_exit_lock[entrys_exits];

int boom_gate_entry_signaled[entrys_exits];
int boom_gate_exit_signaled[entrys_exits];


int main(int argc, char** argv)
{
    entry_queue = createQueue();
    incarpark_queue = createQueue();
    exit_queue = createQueue();
    
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

        boom_gate_entry_signaled[i] = 0;
        boom_gate_exit_signaled[i] = 0;
    }

    pthread_create(&car_init_thread, NULL, (void*) car_queuer, NULL);

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

    pthread_join(car_init_thread, NULL);
    
    for (int i = 0; i < entrys_exits; i++)
        pthread_join(boom_gate_entry_thread[i], NULL);

    for (int i = 0; i < entrys_exits; i++)
        pthread_join(boom_gate_exit_thread[i], NULL);


    
    printRego(entry_queue->front);
    printf("--------------------------------------------------------------------\n");
    printRego(incarpark_queue->front);
    printf("--------------------------------------------------------------------\n");
    printRego(exit_queue->front);
    printf("====================================================================\n");
    

    pthread_mutex_destroy(&lock_queue);

    for (int i = 0; i < entrys_exits; i++)
    {
        pthread_cond_destroy(&boom_gate_entry_signal[i]);
        pthread_mutex_destroy(&boom_gate_entry_lock[i]);
        pthread_cond_destroy(&boom_gate_exit_signal[i]);
        pthread_mutex_destroy(&boom_gate_exit_lock[i]);
    }

    return 0;
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
        {
            // signal that new car has been added to specific entry
            boom_gate_entry_signaled[new_car->entry] ++;
            pthread_mutex_lock(&boom_gate_entry_lock[new_car->entry]);
            pthread_cond_signal(&boom_gate_entry_signal[new_car->entry]);
            pthread_mutex_unlock(&boom_gate_entry_lock[new_car->entry]);

            return new_car;
        }

        else if (!(findCarRego(entry_queue, new_car->rego)) || !(findCarRego(entry_queue, new_car->rego)) || 
            !(findCarRego(entry_queue, new_car->rego)))
        {
            // signal that new car has been added to specific entry
            boom_gate_entry_signaled[new_car->entry] ++;
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
    for(int i = 0; i < 2000; i++)
    {
        // initialise new car and add to queue
        addCar(entry_queue, car_init());
        //printf("entry: %d, carpark: %d, exit: %d\n", listCount(entry_queue), listCount(incarpark_queue), listCount(exit_queue));
        // sleep for 1 - 100 ms
        usleep(((rand() % 100) + 1) * 1000);    
    }

    return NULL;
}

void *boom_gate_entry(void *ptr)
{
    
    int entry = *((int *)ptr);
    //int entry = 2;
    //printf("%d @ 2\n", entry);

    // loop infinately
    for(;;)
    {
        car_t* curr_car;

        // waits to be signaled that car is at entry
        pthread_mutex_lock(&boom_gate_entry_lock[entry]);
        while(boom_gate_entry_signaled == 0)
            pthread_cond_wait(&boom_gate_entry_signal[entry], &boom_gate_entry_lock[entry]);
        pthread_mutex_unlock(&boom_gate_entry_lock[entry]);

        usleep(2000); // wait for car

        curr_car =  findFirstCarEntry(entry_queue, entry);
        
        if ((curr_car != NULL))
        {
            // *** PASS REGO TO CORRECT LPR
            
            if (false)
            {
                removeCarRego(&(entry_queue->front), curr_car);
            }
            else if (true)
            {
                usleep(10000); // open boom gate
                
                pthread_mutex_lock(&lock_queue);
                addCar(incarpark_queue, curr_car);
                removeCarRego(&(entry_queue->front), curr_car);
                pthread_mutex_unlock(&lock_queue);

                /* CREATE NEW CAR THREAD */
                pthread_t car;
                void * arg = malloc(sizeof(car_t));
                arg = curr_car;
                pthread_create(&car, NULL, (void*) car_movement, arg); 
                
                usleep(10000); // close boom gate
                
            }
        }
        if ((entry_queue->front == NULL) && (incarpark_queue->front == NULL) && 
            (exit_queue->front == NULL))
        { 
            break;
        }
        
    }
    return NULL;
}

void *boom_gate_exit(void *ptr)
{
    int exit = *((int *)ptr);
    bool run_once = false;

    // loop infinately
    for(;;)
    {
        car_t* curr_car;

        // waits to be signaled that car is at entry
        pthread_mutex_lock(&boom_gate_exit_lock[exit]);
        while(boom_gate_exit_signaled == 0)
            pthread_cond_wait(&boom_gate_exit_signal[exit], &boom_gate_exit_lock[exit]);
        pthread_mutex_unlock(&boom_gate_exit_lock[exit]);

        usleep(2000); // wait for car+
        curr_car =  findFirstCarEntry(exit_queue, exit);

        
        if (curr_car != NULL)
        {
            // *** PASS REGO TO CORRECT LPR
            usleep(10000); // open boom gate
            
            pthread_mutex_lock(&lock_queue);
            removeCarRego(&(exit_queue->front), curr_car);
            pthread_mutex_unlock(&lock_queue);
            
            usleep(10000); // close boom gate     
        }

        if ((entry_queue->front == NULL) && (incarpark_queue->front == NULL) && 
            (exit_queue->front == NULL))
        {
            break;
        }
    }
}

void *car_movement(void *aCar)
{
    
    car_t* currCar= ((car_t *)aCar);
    usleep(10000); // travel to parking

    usleep(((rand() % 900) + 101) * 1000);

    currCar->exit = rand() % entrys_exits;

    usleep(10000); // travel to exit

    pthread_mutex_lock(&lock_queue);
    addCar(exit_queue, currCar);
    removeCarRego(&(incarpark_queue->front), currCar);
    pthread_mutex_unlock(&lock_queue);

    boom_gate_exit_signaled[currCar->exit] ++;
    pthread_mutex_lock(&boom_gate_exit_lock[currCar->exit]);
    pthread_cond_signal(&boom_gate_exit_signal[currCar->exit]);
    pthread_mutex_unlock(&boom_gate_exit_lock[currCar->exit]);

    return 0;
}