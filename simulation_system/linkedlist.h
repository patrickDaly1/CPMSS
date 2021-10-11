#include <stdbool.h>
#include <math.h>

/**
 * struct that holds relevant info for car 
 */
typedef struct car 
{
    char *rego[6];
    bool entered;
    int parking;
    int entry;
} car_t;

/* node in linked list that holds a car struct */
typedef struct node node_t;
struct node
{
    car_t *car;
    node_t *next;
};

/* The queue, front stores the front node of LL and rear stores the last node of LL */
typedef struct queue queue_t;
struct queue {
    struct node* front, * rear;
};

/* A utility function to create a new linked list node. */
struct node* newNode(car_t* car);

/* A utility function to create an empty queue */
struct queue* createQueue();

/* add new car to the end of queue */
void *addCar(struct queue* q, car_t *car); // *** should this not be a queue_t ?? ***

/* remove car from the front of the queue */
void *removeCar(struct queue* q); // *** should this not be a queue_t ?? ***

/* find car in list given car rego */
//TODO - IMPLEMENT
//node_t *findCar(queue_t q, char *rego);


/**
 * prints rego of all the cars in linked list
 */
void printRego(node_t *head);
