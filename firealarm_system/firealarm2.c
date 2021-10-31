#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "../shared_memory/sharedMemory.h"

int shm_fd;
const char* key = "PARKING";
shm* shared_mem;
size_t shmSize = 2920;

bool alarm_active = false;
int runs = 1;
int num_levels = 5;
int exits = 5;
int entrances = 5;
int medianList[30];

void* tempmonitor(void* ptr) {

	int thread = *((int*)ptr);

	/* Monitor Temperatures */
	for (;;) {

		int tempList[35];
		int medianList[30];
		int count = 0;
		int addr;
		int temp;
		int medianTemp;
		int hightemps;
		int fixedTempCount;

		bool first = true;
		int iterations = 0;

		while (count < 31) {
			printf("count: %d\n", count);
			
			temp = shared_mem->levels[thread].tempSen1;
			//printf("temp: %d sensor:%d\n", temp, i);

			if (first) {
				for (int i = 0; i < num_levels; i++) {
					tempList[i] = temp;
				}
			}
			else {
				tempList[count] = temp;
				count++;
			}

			first = false;

			/* Sort temp list using selection sort */
			int n = sizeof(tempList) / sizeof(tempList[0]);
			int idx, jdx, min_idx;

			for (idx = 0; idx < n - 1; idx++)
			{
				// Find the minimum element in unsorted array
				min_idx = idx;
				for (jdx = idx + 1; jdx < n; jdx++)
					if (tempList[jdx] < tempList[min_idx])
						min_idx = jdx;

				// Swap the found minimum element with the first element
				int temp = tempList[min_idx];
				tempList[min_idx] = tempList[idx];
				tempList[idx] = temp;
			}

			/* Find median */
			medianTemp = tempList[count - 3];
			

			medianList[iterations] = medianTemp;
			iterations++;
			//printf("median temp: %d at index %d \n", medianList[j], j);
		}
		
		/* Calculate fixed temperature fire detection */
		fixedTempCount = 0;
		for (int i = 0; i < 30; i++) {
			//printf("median temp: %d \n", medianList[i]);
			if (medianList[i] >= 58) {
				fixedTempCount++;
			}
		}

		printf("Fixed temp count: %d\n", fixedTempCount);

		if (fixedTempCount >= 27) {
			alarm_active = true;
		}

		/* Calculate rate of rise fire detection */
		if ((medianList[30] - medianList[0]) > 8) {
			alarm_active = true;
		}

		printf("alarm: %d\n", alarm_active);

		usleep(2000);
	}
}


int main()
{
	//Locate the segment
	if ((shm_fd = shm_open(key, O_RDWR, 0)) < 0)
	{
		perror("shm_open");
		return 1;
	}

	//Attach segment to our data space.
	if ((shared_mem = (shm*)mmap(0, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (void*)-1)
	{
		perror("mmap");
		return 1;
	}

	pthread_t threads[num_levels];

	int level[num_levels];
	for (int i = 0; i < num_levels; i++) {

		level[i] = i;
		pthread_create(&threads[i], NULL, tempmonitor, &level[i]);
	}

	for (;;) {
		runs++;

		/* Activate Alarm */
		if (alarm_active) {
			fprintf(stderr, "*** ALARM ACTIVE ***\n");

			/* Handle the alarm system and open boom gates
			   Activate alarms on all levels */
			for (int i = 0; i < num_levels; i++) {
				shared_mem->levels[i].alarm1 = true;
			}

			/* Show evacuation message */
			char* evacmessage = "EVACUATE ";
			for (char* p = evacmessage; *p != '\0'; p++) {
				for (int i = 0; i < entrances; i++) {
					pthread_mutex_lock(&shared_mem->entrances[i].SIGN.lock);

					shared_mem->entrances[i].SIGN.display = *p;

					pthread_mutex_unlock(&shared_mem->entrances[i].SIGN.lock);
				}
				usleep(20000);
			}
		}
		usleep(1000);
	}

	munmap(shared_mem, 2920);
	close(shm_fd);
}
