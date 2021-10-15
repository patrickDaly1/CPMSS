#include <stdbool.h>
#include <math.h>

/**
 * struct that holds relevant info for car 
 */
typedef struct car 
{
    char *rego[6];
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
void addCar(struct queue* q, car_t *car); // *** should this not be a queue_t ?? ***

/* remove car from the front of the queue */
void removeCar(struct queue* q); 

/* find if there exists a car with a given rego */
bool findCarRego(struct queue* q, char rego[6]);

/* remove car based on rego given */
node_t removeCarRego(node_t* head, car_t* car);

/* return a car based on an entry number */
car_t* findFirstCarEntry(struct queue* q, int entry);

/* return a car based on an entry number */
car_t* findFirstCarExit(struct queue* q, int exit);

/**
 * prints rego of all the cars in linked list
 */
void printRego(node_t *head);
