#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#define NUM_ENTRANCES 5
#define NUM_EXITS 5
#define NUM_LEVELS 5

//boomgate
typedef struct boom_gate {
    //pthread_mutex_t for boom gate
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char status;
} boom_gate_t;

//LPR
typedef struct lpr {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char rego[6];
} lpr_t;

//Information Sign
typedef struct sign {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char display;
} sign_t;

//entrance
typedef struct entrance {
    lpr_t LPR;
    boom_gate_t BG;
    sign_t SIGN;
} entrance_t;

//exit
typedef struct exit {
    lpr_t LPR;
    boom_gate_t BG;
} exit_t;

//level
typedef struct level {
    lpr_t LPR;
    boom_gate_t BG;
    short tempSen1; //correct?
    bool alarm1; //correct?
} level_t;

//Shared memory
typedef struct shm {
    entrance_t entrances[NUM_ENTRANCES];
    exit_t exits[NUM_EXITS];
    level_t levels[NUM_LEVELS];
} shm;
