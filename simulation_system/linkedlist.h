#include <stdbool.h>
#include <math.h>

/**
 * To - Do
 * 
 * remove function is not working properly at all
 */

/**
 * struct that holds relevant info for car 
 */
typedef struct car 
{
    char rego[6];
    int parking;
    int entry;
    int exit;
} car_t;

/* node in linked list that holds a car struct */
typedef struct node node_t;
struct node
{
    car_t *car;
    node_t *next;
};

/* The queue, front stores the front node of LL and rear stores the last node of LL */
typedef struct queue {
    struct node *front, *rear;
}queue_t;

/* A utility function to create a new linked list node. */
node_t* newNode(car_t* car);

/* A utility function to create an empty queue */
queue_t* createQueue();

/* add new car to the end of queue */
void addCar(queue_t* q, car_t *car); // *** should this not be a queue_t ?? ***

/* remove car from the front of the queue */
void removeCar(queue_t* q); 

/* find if there exists a car with a given rego */
bool findCarRego(queue_t* q, char *rego);

int listCount(queue_t* q);

/* remove car based on rego given */
void removeCarRego(node_t** head, car_t* car);

/* return a car based on an entry number */
car_t* findFirstCarEntry(queue_t* q, int entry);

/* return a car based on an entry number */
car_t* findFirstCarExit(queue_t* q, int exit);

/**
 * prints rego of all the cars in linked list
 */
void printRego(node_t *head);
