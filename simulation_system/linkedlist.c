#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "linkedlist2.h"

/* Given a reference (pointer to pointer) to the head
   of a list and an int, appends a new node at the end  */
void append(struct Node** head_ref, struct car* new_data)
{
    /* 1. allocate node */
    struct Node* new_node = (struct Node*) malloc(sizeof(struct Node));
 
    struct Node *last = *head_ref;  /* used in step 5*/
  
    /* 2. put in the data  */
    new_node->data  = new_data;
 
    /* 3. This new node is going to be the last node, so make next
          of it as NULL*/
    new_node->next = NULL;
 
    /* 4. If the Linked List is empty, then make the new node as head */
    if (*head_ref == NULL)
    {
       *head_ref = new_node;
       return;
    } 
      
    /* 5. Else traverse till the last node */
    while (last->next != NULL)
        last = last->next;
  
    /* 6. Change the next of last node */
    last->next = new_node;
    return;   
}

void deleteNode(struct Node** head_ref, char* key)
{
    // Store head node
    struct Node *temp = *head_ref, *prev;
    // If head node itself holds the key to be deleted
    if (temp != NULL && (strcmp(temp->data->rego, key) == 0)) {
        *head_ref = temp->next; // Changed head
        free(temp); // free old head
        return;
    }
 
    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && (strcmp(temp->data->rego, key) != 0)) {
        prev = temp;
        temp = temp->next;
    }
 
    // If key was not present in linked list
    if (temp == NULL)
        return;
 
    // Unlink the node from linked list
    prev->next = temp->next;
 
    free(temp); // Free memory
}

// This function prints contents of linked list starting from head
void printList(struct Node *node)
{
    while (node != NULL)
    {
        printf(" %s \n", node->data->rego);
        node = node->next;
    }
}

int listCount(struct Node *node)
{
    int count = 0;
    while (node != NULL)
    {
        count++;
        node = node->next;
    }
    return count;
}

car_t* searchEntry(struct Node* head, int x)
{
    struct Node* current = head;  // Initialize current
    while (current != NULL)
    {
        if (current->data->entry == x)
            return current->data;
        current = current->next;
    }
    return NULL;
}

car_t* searchExit(struct Node* head, int x)
{
    struct Node* current = head;  // Initialize current
    while (current != NULL)
    {
        if (current->data->exit == x)
            return current->data;
        current = current->next;
    }
    return NULL;
}

bool search(struct Node* head, char* x)
{
    struct Node* current = head;  // Initialize current
    while (current != NULL)
    {
        if ((strcmp(current->data->rego, x) == 0))
            return true;
        current = current->next;
    }
    return false;
}