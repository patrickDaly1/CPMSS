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
    htab_add(&h, "fakeR1", 0, 1);
    htab_add(&h, "fakeR2", 0, 2);
    htab_add(&h, "fakeR3", 0, 3);
    printf("1st test: \n");
    htab_print(&h);

    //Setup file reader
    FILE *fp;
    size_t len = 10;
    char *line;
    size_t read;
    if((fp = fopen("plates.txt", "r")) == NULL) {
        perror("fopen\n");
        exit(1);
    }
    if((line = (char *)malloc(len * sizeof(char))) == NULL) {
        perror("Unable to allocate buffer\n");
        exit(2);
    }

    //read plates.txt per line and store in hashtable
    printf("\n2nd test: \n");
    int level = 1;
    while((read = getline(&line, &len, fp)) != -1) {
        char copy[read];
        strncpy(copy, line, read - 2);
        htab_add(&h, copy, 0, level);
        if(level >= 5) {    
            level = 0;
        } else {
            ++level;
        }
    }

    //close
    fclose(fp);
    free(line);

    htab_print(&h);

    return 0;
}