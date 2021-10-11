#include <stdbool.h>
#include <math.h>

/* struct that holds relevant info for car */
typedef struct car 
{
    char *rego;
    bool entered;
    int parking;
    int entry;
} car_t;

/* node in linked list that holds car */
typedef struct node node_t;
struct node
{
    car_t *person;
    node_t *next;
};

/* add new car to the end of linked list */
node_t *node_add(node_t *head, car *car);

/* remove car from the front of linked list */
node_t *node_delete(node_t *head, char *rego);

/* find car in list given car rego */
node_t *find_car(node_t *head, char *rego);

/** 
 * initialises a new car to be added to the simulator
 *
 * 1. generate random new rego that isnt in currently in the system
 * 2. generate numer to determine which entry car should go to
 * 
*/
car_t car_init(int );
