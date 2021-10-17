#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

long getTimeMilli() {
    long ms; // Milliseconds
    long s;  // Seconds
    struct timespec spec;

    if(clock_gettime(CLOCK_MONOTONIC, &spec) < 0) {
        //issue setting up time
        perror("clock_gettime\n");
        exit(1);
    }

    s  = spec.tv_sec * 1.0e3; // convert to milliseconds
    ms = spec.tv_nsec / 1.0e6; // convert nanoseconds to milliseconds

    return ms + s;
}

void *myThreadFun(void *arg) {
    printf("Current time: %03ld milliseconds\n", getTimeMilli());
    return NULL;
}

void print_current_time_with_ms (void)
{
    long start = getTimeMilli();
    printf("Internal time start : %03ld milliseconds\n", start);
    sleep(10);
    long end = getTimeMilli();
    printf("Internal time end: %03ld milliseconds\n", getTimeMilli());
    printf("Time passed: %03ld\n", end - start);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, myThreadFun, NULL);
    pthread_join(thread_id, NULL);
}

int main(void) {
    print_current_time_with_ms();
    return 0;
}