#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

/** Shared memory struct */
typedef struct shm {
    //Entrance #1
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock1;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond1;
    // license plate reading for LPR
    char *lpr_entrance_1; //this suppose to be char lpr_entrance_1[length] instead?
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock1;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond1;
    // status character for boom gate
    char bg_entrance_status1;
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock1;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond1;
    // character display for information sign
    char is_entrance_display1;

    //Entrance #2
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock2;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond2;
    // license plate reading for LPR
    char *lpr_entrance_2;
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock2;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond2;
    // status character for boom gate
    char bg_entrance_status2;
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock2;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond2;
    // character display for information sign
    char is_entrance_display2;


    //Entrance #3
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock3;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond3;
    // license plate reading for LPR
    char *lpr_entrance_3;
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock3;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond3;
    // status character for boom gate
    char bg_entrance_status3;
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock3;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond3;
    // character display for information sign
    char is_entrance_display3;


    //Entrance #4
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock4;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond4;
    // license plate reading for LPR
    char *lpr_entrance_4;
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock4;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond4;
    // status character for boom gate
    char bg_entrance_status4;
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock4;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond4;
    // character display for information sign
    char is_entrance_display4;


    //Entrance #5
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock5;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond5;
    // license plate reading for LPR
    char *lpr_entrance_5;
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock5;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond5;
    // status character for boom gate
    char bg_entrance_status5;
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock5;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond5;
    // character display for information sign
    char is_entrance_display5;




    //Exit #1
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock1;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond1;
    // license plate reading for LPR
    char *lpr_exit_1;
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock1;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond1;
    // status character for boom gate
    char bg_exit_status1;


    //Exit #2
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock2;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond2;
    // license plate reading for LPR
    char *lpr_exit_2;
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock2;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond2;
    // status character for boom gate
    char bg_exit_status2;


    //Exit #3
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock3;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond3;
    // license plate reading for LPR
    char *lpr_exit_3;
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock3;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond3;
    // status character for boom gate
    char bg_exit_status3;

    //Exit #4
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock4;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond4;
    // license plate reading for LPR
    char *lpr_exit_4;
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock4;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond4;
    // status character for boom gate
    char bg_exit_status4;

    //Exit #5
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock5;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond5;
    // license plate reading for LPR
    char *lpr_exit_5;
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock5;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond5;
    // status character for boom gate
    char bg_exit_status5;


    //Level #1
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_level_lock1;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_level_cond1;
    // license plate reading for LPR
    char *lpr_level_1;
    // temperature sensor (just temperatue)
    short tempSen1;
    // fire alarm
    bool alarm1;

    //Level #2
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_level_lock2;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_level_cond2;
    // license plate reading for LPR
    char *lpr_level_2;
    // temperature sensor (just temperatue)
    short tempSen2;
    // fire alarm
    bool alarm2;

    //Level #3
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_level_lock3;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_level_cond3;
    // license plate reading for LPR
    char *lpr_level_3;
    // temperature sensor (just temperatue)
    short tempSen3;
    // fire alarm
    bool alarm3;

    //Level #4
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_level_lock4;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_level_cond4;
    // license plate reading for LPR
    char *lpr_level_4;
    // temperature sensor (just temperatue)
    short tempSen4;
    // fire alarm
    bool alarm4;

    //Level #5
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_level_lock5;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_level_cond5;
    // license plate reading for LPR
    char *lpr_level_5;
    // temperature sensor (just temperatue)
    short tempSen5;
    // fire alarm
    bool alarm5;

} shm;
