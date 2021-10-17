#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "hashtable.h"
#include "../shared_memory/sharedMemory.h"
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>


//Read all values from plates.txt then display
int main(void) {
    //setup hashtable
    size_t buckets = 10;
    htab_t h;
    if (!htab_init(&h, buckets))
    {
        printf("failed to initialise hash table\n");
        return EXIT_FAILURE;
    }
    
    // //demo
    // htab_add(&h, "fakeRego1", 0, 1);
    // htab_add(&h, "fakeRego2", 0, 2);
    // htab_add(&h, "fakeRego3", 0, 3);
    // printf("1st test: \n");
    // htab_print(&h);

    //Setup file reader
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    if((fp = fopen("plates.txt", "r")) == NULL) {
        perror("fopen\n");
        exit(1);
    }

    //read plates.txt per line and store in hashtable
    printf("\n2nd test: \n");
    int level = 1;
    while((read = getline(&line, &len, fp)) != -1) {
        char copy[strlen(line) + 3];
        // char *copy = line;
        strncpy(copy, line, strlen(line) - 2);
        htab_add(&h, copy, 0, level);
        if(level >= 5) {
            level = 0;
        } else {
            ++level;
        }
    }
    htab_print(&h);

    //close
    fclose(fp);
    free(line);

    return 0;
}