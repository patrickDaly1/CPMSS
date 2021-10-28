#include <inttypes.h> // for portable integer declarations
#include <stdbool.h>  // for bool type
#include <stdio.h>    // for printf()
#include <stdlib.h>   // for malloc(), free(), NULL
#include <string.h>   // for strcmp()
#include <time.h>
#include <math.h>

typedef struct item item_t;
struct item
{
    char rego[6];
    long timeEntered;
    int levelParked;
    item_t *next;
};

void item_print(item_t *i)
{
    printf("Rego=%s timeEntered=%03ld levelParked =%d\n", i->rego, i->timeEntered, i->levelParked);
}

// A hash table mapping a string to an integer.
typedef struct htab htab_t;
struct htab
{
    item_t **buckets;
    size_t size;
};

// Initialise a new hash table with n buckets.
// pre: true
// post: (return == false AND allocation of table failed)
//       OR (all buckets are null pointers)
bool htab_init(htab_t *h, size_t n)
{
    h->size = n;
    h->buckets = (item_t **)calloc(n, sizeof(item_t *));
    return h->buckets != 0;
}

// The Bernstein hash function.
// A very fast hash function that works well in practice.
size_t djb_hash(char *s)
{
    size_t hash = 5381;
    int c;
    while ((c = *s++) != '\0')
    {
        // hash = hash * 33 + c
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Calculate the offset for the bucket for rego in hash table.
size_t htab_index(htab_t *h, char *rego)
{
    return djb_hash(rego) % h->size;
}

// Find pointer to head of list for rego in hash table.
item_t *htab_bucket(htab_t *h, char *rego)
{
    return h->buckets[htab_index(h, rego)];
}

// Find an item for rego in hash table.
// pre: true
// post: (return == NULL AND item not found)
//       OR (strcmp(return->rego, rego) == 0)
item_t *htab_find(htab_t *h, char *rego)
{
    for (item_t *i = htab_bucket(h, rego); i != NULL; i = i->next)
    {
        // printf("rego from htab in htab_find: %s\n", i->rego);
        //iterate over and compare chars
        // bool checkRego = true;
        // for(int x = 0; x < 6; ++x) {
        //     if(rego[x] != (i->rego)[x]) {
        //         checkRego = false;
        //     }
        //     if (!checkRego) {
        //         break;
        //     }
        // }
        // if(checkRego) {
        //     return i;
        // }
        if (strcmp(i->rego, rego) == 0)
        { // found the key
            return i;
        }
    }
    return NULL;
}

// Add a rego with value to the hash table.
// pre: htab_find(h, rego) == NULL
// post: (return == false AND allocation of new item failed)
//       OR (htab_find(h, rego) != NULL) 
bool htab_add(htab_t *h, char *rego, long timeEntered, int levelParked)
{
    // allocate new item
    item_t *newhead = (item_t *)malloc(sizeof(item_t));
    if (newhead == NULL)
    {
        return false;
    }
    strcpy(newhead->rego, rego);
    newhead->timeEntered = timeEntered;
    newhead->levelParked = levelParked;

    // hash key and place item in appropriate bucket
    size_t bucket = htab_index(h, rego);
    newhead->next = h->buckets[bucket];
    h->buckets[bucket] = newhead;
    return true;
}

// Print the hash table.
// pre: true
// post: hash table is printed to screen
void htab_print(htab_t *h)
{
    printf("hash table with %lu buckets\n", h->size);
    for (size_t i = 0; i < h->size; ++i)
    {
        printf("bucket %lu: \n", i);
        if (h->buckets[i] == NULL)
        {
            printf("empty\n");
        }
        else
        {
            for (item_t *j = h->buckets[i]; j != NULL; j = j->next)
            {
                item_print(j);
                if (j->next != NULL)
                {
                    printf(" -> ");
                }
            }
            //printf("\n");
        }
    }
}

// Delete an item with rego from the hash table.
// pre: htab_find(h, rego) != NULL
// post: htab_find(h, rego) == NULL
void htab_delete(htab_t *h, char *rego)
{
    item_t *head = htab_bucket(h, rego);
    item_t *current = head;
    item_t *previous = NULL;
    while (current != NULL)
    {
        if (strcmp(current->rego, rego) == 0)
        {
            if (previous == NULL)
            { // first item in list
                h->buckets[htab_index(h, rego)] = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            free(current);
            break;
        }
        previous = current;
        current = current->next;
    }
}

// Destroy an initialised hash table.
// pre: htab_init(h)
// post: all memory for hash table is released
void htab_destroy(htab_t *h)
{
    // free linked lists
    for (size_t i = 0; i < h->size; ++i)
    {
        item_t *bucket = h->buckets[i];
        while (bucket != NULL)
        {
            item_t *next = bucket->next;
            free(bucket);
            bucket = next;
        }
    }

    // free buckets array
    free(h->buckets);
    h->buckets = NULL;
    h->size = 0;
}

void htab_change_time(htab_t *h, char *rego, long time) {
    item_t *i = htab_find(h, rego);
    if(i != NULL) {
        //found, change time
        i->timeEntered = time;
    }
}

// TESTER FUNCTION

// int main(int argc, char **argv)
// {
//     // create a hash table with 10 buckets
//     printf("creating hash table:\n");
//     size_t buckets = 10;
//     htab_t h;
//     if (!htab_init(&h, buckets))
//     {
//         printf("failed to initialise hash table\n");
//         return EXIT_FAILURE;
//     }

//     // add items to hash table and print
//     struct timespec start, stop;
//     htab_add(&h, "123ABC", clock_gettime( CLOCK_REALTIME, &start), 2);
//     htab_add(&h, "456JCD", clock_gettime( CLOCK_REALTIME, &start), 1); // violate pre-condition to get two hello's in table
//     htab_add(&h, "SDF123", time(0), 2);
//     htab_print(&h);
//     printf("\n");

//     // find item in hash table
//     printf("find item in hash table:\n");
//     item_print(htab_find(&h, "456JCD"));
//     printf("\n\n");

//     // delete items from hash table and print
//     printf("deleting items from hash table:\n");
//     htab_delete(&h, "123ABC");
//     htab_delete(&h, "SDF123");
//     htab_print(&h);
//     printf("\n");

//     // clean up hash table
//     htab_destroy(&h);
// }