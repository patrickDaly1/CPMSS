#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "linkedlist.h"

node_t* newNode(car_t* car)
{
    node_t* temp = (node_t*)malloc(sizeof(node_t));
    temp->car = car;
    temp->next = NULL;
    return temp;
}

queue_t* createQueue()
{
    queue_t* q = (queue_t*)malloc(sizeof(queue_t));
    q->front = q->rear = NULL;
    return q;
}

void addCar(queue_t* q, car_t* car)
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

void removeCar(queue_t* q)
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

bool findCarRego(queue_t* q, char* rego)
{
    node_t* temp;
    temp = q->front;

    while (temp != NULL)
    {
        if (strcmp(temp->car->rego, rego) == 0)
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

        if (strcmp(current->car->rego, car->rego) == 0)
        {
            node_t* newhead = head;
            if (previous == NULL) // first item in list
                newhead = current->next;
            else
            {
                if (current->next == NULL) // last item in list
                    if (previous !=NULL)
                    {
                        free(current);
                        previous->next = NULL;
                        return previous;
                    }
                    else
                    {
                        free(current);
                        return head;
                    }
                
                previous->next = current->next;
            }

            free(current);
            return newhead;
        }
        previous = current;

        current = current->next;
    }

    return head;
}

// q->start = removeCarRego(q, car);

car_t* findFirstCarEntry(queue_t* q, int entry)
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

car_t* findFirstCarExit(queue_t* q, int exit)
{
    node_t* temp;
    temp = q->front;

    while (temp != NULL)
    {
        if (temp->car->exit == exit)
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
        printf("%s\n", head->car->rego);
    }
}