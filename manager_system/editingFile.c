#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <string.h>

int main(void) {
    FILE *fp = fopen("billing.txt", "a");
    if(fp == NULL) {
        perror("fopen billing\n");
        return 1;
    }
    long num = 92393.239372372832;
    fprintf(fp, "%s %.2f\n", "029MZH", (double)num);
    long num1 = 932703278;
    double num2 = num1 * 0.05;
    printf("%.2f\n", num2);
    return 0;
}
