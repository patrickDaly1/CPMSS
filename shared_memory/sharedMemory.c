#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

/** Shared memory struct */
typedef struct PARKING {
    //Entrance #1
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock1;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond1;
    // license plate reading for LPR
    char *lpr_entrance_1;
    // padding?
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock1;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond1;
    // status character for boom gate
    char bg_entrance_status1;
    //padding?
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock1;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond1;
    // character display for information sign
    char is_entrance_display1;
    //padding?

    //Entrance #2
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock2;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond2;
    // license plate reading for LPR
    char *lpr_entrance_2;
    // padding?
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock2;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond2;
    // status character for boom gate
    char bg_entrance_status2;
    //padding?
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock2;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond2;
    // character display for information sign
    char is_entrance_display2;
    //padding?

    //Entrance #3
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock3;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond3;
    // license plate reading for LPR
    char *lpr_entrance_3;
    // padding?
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock3;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond3;
    // status character for boom gate
    char bg_entrance_status3;
    //padding?
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock3;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond3;
    // character display for information sign
    char is_entrance_display3;
    //padding?

    //Entrance #4
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock4;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond4;
    // license plate reading for LPR
    char *lpr_entrance_4;
    // padding?
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock4;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond4;
    // status character for boom gate
    char bg_entrance_status4;
    //padding?
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock4;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond4;
    // character display for information sign
    char is_entrance_display4;
    //padding?

    //Entrance #5
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_entrance_lock5;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_entrance_cond5;
    // license plate reading for LPR
    char *lpr_entrance_5;
    // padding?
    //pthread_mutex_t for boom gate
    pthread_mutex_t bg_entrance_lock5;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_entrance_cond5;
    // status character for boom gate
    char bg_entrance_status5;
    //padding?
    // pthread_mutex_t for information sign
    pthread_mutex_t is_entrance_lock5;
    // pthread_cond_t for information sign
    pthread_cond_t is_entrance_cond5;
    // character display for information sign
    char is_entrance_display5;
    //padding?



    //Exit #1
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock1;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond1;
    // license plate reading for LPR
    char *lpr_exit_1;
    // padding?
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock1;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond1;
    // status character for boom gate
    char bg_exit_status1;
    // padding?

    //Exit #2
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock2;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond2;
    // license plate reading for LPR
    char *lpr_exit_2;
    // padding?
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock2;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond2;
    // status character for boom gate
    char bg_exit_status2;
    // padding?


    //Exit #3
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock3;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond3;
    // license plate reading for LPR
    char *lpr_exit_3;
    // padding?
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock3;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond3;
    // status character for boom gate
    char bg_exit_status3;
    // padding?

    //Exit #4
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock4;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond4;
    // license plate reading for LPR
    char *lpr_exit_4;
    // padding?
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock4;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond4;
    // status character for boom gate
    char bg_exit_status4;
    // padding?

    //Exit #5
    // pthread_mutex_t for LPR for Exit
    pthread_mutex_t lpr_exit_lock5;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_exit_cond5;
    // license plate reading for LPR
    char *lpr_exit_5;
    // padding?
    // pthread_mutex_t for boom gate
    pthread_mutex_t bg_exit_lock5;
    // pthread_cond_t for boom gate
    pthread_cond_t bg_exit_cond5;
    // status character for boom gate
    char bg_exit_status5;
    // padding?



    //Level #1
    // pthread_mutex_t for LPR
    pthread_mutex_t lpr_level_lock1;
    // pthread_cond_t for LPR
    pthread_cond_t lpr_level_cond1;
    // license plate reading for LPR
    char *lpr_level_1;
    // padding
    // temperature sensor
    
    // fire alarm

    // padding?
    //Level #2


    //Level #3


    //Level #4


    //Level #5




} PARKING;
