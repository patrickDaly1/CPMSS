#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>


struct boomgate {
	pthread_mutex_t m;
	pthread_cond_t c;
	char s;
};

struct parkingsign {
	pthread_mutex_t m;
	pthread_cond_t c;
	char display;
};

void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

/* Source: https://www.geeksforgeeks.org/selection-sort/ */
void selectionSort(int arr[], int n)
{
    int i, j, min_idx;
 
    // One by one move boundary of unsorted subarray
    for (i = 0; i < n-1; i++)
    {
        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i+1; j < n; j++)
        if (arr[j] < arr[min_idx])
            min_idx = j;
 
        // Swap the found minimum element with the first element
        swap(&arr[min_idx], &arr[i]);
    }
}


int main()
{
	int runs = 1;
	int levels = 5;
	int exits = 5;
	int entrances = 5;
	
	int medianList[30];

    int shm_fd;
    void *shm;
	
	shm_fd = shm_open("PARKING", O_RDWR, 0);
	shm = mmap(0, 2920, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

	
	while(runs < 10) {

		int tempList[35];
		int count = 0;
		int addr;
		int temp;
		int medianTemp;
		int hightemps;
		int fixedTempCount;

		bool alarm_active = false;

		runs++;

		for(int i = 0; i < levels; i++) {

			/* Monitor Temperatures */

			/* Calculate address of temperature sensor */
			addr = 0150 * i + 2496;

			bool first = true;

			int j = 0;

			while(count != 30) {
				j++;

				if(first) {
					for(int i = 0; i < 5; i++) {
						temp = *((int16_t *)(shm + addr));
						tempList[i] = temp;
						count++;
						sleep(2);
					}
				} else {

					temp = *((int16_t *)(shm + addr));
					tempList[count] = temp;
					count++;
					sleep(2);
				}

				first = false;

				/* Sort temp list */
    			int n = sizeof(tempList)/sizeof(tempList[0]);
    			selectionSort(tempList, n);

				/* Find median */
				medianTemp = tempList[count - 3];
				
				medianList[j] = medianTemp;
			}

			/* Calculate fixed temperature fire detection */
			fixedTempCount = 0;
			for(int i = 0; i < 30; i++) {
				if(medianList[i] > 58) {
					fixedTempCount++;
				}
			}
			
			if(fixedTempCount >= 27) {
				alarm_active = true;
			}

			/* Calculate rate of rise fire detection */
			if((medianList[30] - medianList[0]) > 8) {
				alarm_active = true;
			}

			/* Activate Alarm */
			if (alarm_active) {
				fprintf(stderr, "*** ALARM ACTIVE ***\n");
		
				/* Handle the alarm system and open boom gates
				   Activate alarms on all levels */
				for (int i = 0; i < levels; i++) {
					int addr = 0150 * i + 2498;
					char *alarm_trigger = (char *)shm + addr;
					*alarm_trigger = 1;
				}
				
				/* Open up all boom gates */
				for (int i = 0; i < entrances; i++) {
					int addr = 288 * i + 96;
					struct boomgate *bg = shm + addr;
					
					if (bg->s == 'C') {
						bg->s = 'R';
					}
				}
				for (int i = 0; i < exits; i++) {
					int addr = 192 * i + 1536;
					struct boomgate *bg = shm + addr;
					
					if (bg->s == 'C') {
						bg->s = 'R';
					}
				}
				
				/* Show evacuation message */
				char *evacmessage = "EVACUATE ";
				for (char *p = evacmessage; *p != '\0'; p++) {
					for (int i = 0; i < entrances; i++) {
						int addr = 288 * i + 192;
						struct parkingsign *sign = shm + addr;
						sign->display = *p;
					}
					sleep(20);
				}
			}
		}
	}
	
	munmap((void *)shm, 2920);
	close(shm_fd);
}
