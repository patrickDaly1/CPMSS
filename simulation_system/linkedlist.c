#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
//#include "linkedlist.h"

/**
 * struct that holds relevant info for car
 */
typedef struct car
{
    char* rego[6];
    int parking;
    int entry;
    int exit;
} car_t;

/* node in linked list that holds a car struct */
typedef struct node node_t;
struct node
{
    car_t* car;
    node_t* next;
};

/* The queue, front stores the front node of LL and rear stores the last node of LL */
typedef struct queue queue_t;
struct queue {
    node_t* front, * rear;
};

node_t* newNode(car_t* car)
{
    node_t* temp = (struct node*)malloc(sizeof(struct node));
    temp->car = car;
    temp->next = NULL;
    return temp;
}

struct queue* createQueue()
{
    struct queue* q = (struct queue*)malloc(sizeof(struct queue));
    q->front = q->rear = NULL;
    return q;
}

void addCar(struct queue* q, car_t* car)
{
    // Create a new LL node
    node_t* temp = newNode(car);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

void removeCar(struct queue* q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return;

    // Store previous front and move front one node ahead
    node_t* temp = q->front;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}

bool findCarRego(struct queue* q, char rego[6])
{
    node_t* temp;
    temp = q->front;

    while (temp != NULL)
    {
        if (strcmp(*temp->car->rego, rego) == 0)
        {
            return true;
        }

        temp = temp->next;
    }

    return false;
}

node_t* removeCarRego(node_t* head, car_t* car)
{
    node_t* previous = NULL;
    node_t* current = head;

    while (current != NULL)
    {
        if (strcmp(*current->car->rego, *car->rego) == 0)
        {
            node_t* newhead = head;
            if (previous == NULL) // first item in list
                newhead = current->next;
            else
                previous->next = current->next;
            free(current);
            return newhead;
        }
        previous = current;
        current = current->next;
    }

    return head;
}

// q->start = removeCarRego(q, car);

car_t* findFirstCarEntry(struct queue* q, int entry)
{
    node_t* temp;
    temp = q->front;

    while (temp != NULL)
    {
        if (temp->car->entry == entry)
        {
            return temp->car;
        }

        temp = temp->next;
    }

    return NULL;
}

car_t* findFirstCarExit(struct queue* q, int exit)
{
    node_t* temp;
    temp = q->front;

    while (temp != NULL)
    {
        if (temp->car->entry == exit)
        {
            return temp->car;
        }

        temp = temp->next;
    }

    return NULL;
}


void printRego(node_t* head)
{
    for (; head != NULL; head = head->next)
    {
        printf("%s", *head->car->rego);
    }
}

// Driver Program to test above functions
int main()
{
    queue_t* q = createQueue();

    car_t* car1 = (car_t*)malloc(sizeof(car_t));
    car1->entry = 1;
    car1->exit = 1;
    car1->parking = 1;
    car1->rego[0] = "a";

    car_t* car2 = (car_t*)malloc(sizeof(car_t));
    car2->entry = 2;
    car2->exit = 2;
    car2->parking = 2;
    car2->rego[0] = "b";

    car_t* car3 = (car_t*)malloc(sizeof(car_t));
    car3->entry = 3;
    car3->exit = 3;
    car3->parking = 3;
    car3->rego[0] = "c";

    addCar(q, car1);
    addCar(q, car2);
    addCar(q, car3);

    printRego(q->front);
    printf("\n");

    removeCar(q);

    printRego(q->front);
    printf("\n");

    char* buf = "b";
    if (findCarRego(q, buf))
        printf("car found");
    else
        printf("car not found");

    q->front = removeCarRego(q->front, car1);

    car_t* car4 = findFirstCarEntry(q, 3);
    printf("%d", car4->entry);

    printRego(q->front);
    printf("\n");

    return 0;
}