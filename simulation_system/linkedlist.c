#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
#include "simulator.c"


struct node* newNode(car_t* car)
{
    struct node* temp = (struct node*)malloc(sizeof(struct node));
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
    struct node* temp = newNode(car);

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
    struct node* temp = q->front;

    q->front = q->front->next;

    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    free(temp);
}

void printRego(node_t* head)
{
    for (; head != NULL; head = head->next)
    {
        printf(head->car);
    }
}

// Driver Program to test anove functions
int main()
{
    struct queue* q = createQueue();
    //TODO - TEST
    //struct car* car = 

    //addCar(q, 10);
    //addCar(q, 20);
    //removeCar(q);
    //removeCar(q);
    //addCar(q, 30);
    //addCar(q, 40);
    //addCar(q, 50);
    //removeCar(q);

    printf("Queue Front : %d \n", q->front->car);
    printf("Queue Rear : %d", q->rear->car);

    return 0;
}