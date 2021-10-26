#include <stdbool.h>
#include <math.h>

typedef struct car 
{
    char rego[6];
    int parking;
    int entry;
    int exit;
} car_t;

// A linked list node
struct Node
{
  struct car* data;
  struct Node *next;
};

car_t* searchEntry(struct Node* head, int x);

car_t* searchExit(struct Node* head, int x);

int listCount(struct Node *node);

void printList(struct Node *node);

void deleteNode(struct Node** head_ref, char* key);

void append(struct Node** head_ref, struct car* new_data);

bool search(struct Node* head, char* x);