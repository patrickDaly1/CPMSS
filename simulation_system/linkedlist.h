#include <stdbool.h>
#include <math.h>

/**
 * struct that holds relevant info for car 
 */
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
node_t *node_add(node_t *head, car_t *car);

/* remove car from the front of linked list */
node_t *node_delete(node_t *head, char *rego);

/* find car in list given car rego */
node_t *find_car(node_t *head, char *rego);


/**
 * prints rego of all the cars in linked list
 */
void print_rego(node_t *head);
