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
int num_levels = 5;

void* tempmonitor(void* ptr) {
	int thread = *((int*)ptr);
	int temp;
	temp = shared_mem->levels[thread].tempSen1;

	/* Monitor Temperatures */
	while(temp != 0) {

		int tempList[35];
		int medianList[30];
		int count = 0;
		int addr;
		int medianTemp;
		int hightemps;
		int fixedTempCount;

		bool first = true;
		int iterations = 0;

		/* Evaluate the first five temperatures before smoothing*/
		for (int i = 0; i < num_levels; i++) {
			temp = shared_mem->levels[thread].tempSen1;
			tempList[i] = temp;
		}

		/* Evaluate temps 5-35 for smoothing*/
		for (count = 5; count < 35; count++) {
			temp = shared_mem->levels[thread].tempSen1;
			tempList[count] = temp;

			int temporaryList[5];
			for (int i = 0; i < 5; i++) {
				temporaryList[i] = tempList[count - 5 + i];
				printf("temp: %d \n", temporaryList[i]);
			}

			/* Sort temp list using selection sort */
			int n = sizeof(temporaryList) / sizeof(temporaryList[0]);
			int idx, jdx, min_idx;

			for (idx = 0; idx < n - 1; idx++)
			{
				// Find the minimum element in unsorted array
				min_idx = idx;
				for (jdx = idx + 1; jdx < n; jdx++)
					if (temporaryList[jdx] < temporaryList[min_idx])
						min_idx = jdx;

				// Swap the found minimum element with the first element
				int temp = tempList[min_idx];
				temporaryList[min_idx] = temporaryList[idx];
				temporaryList[idx] = temp;
			}

			/* Find median */
			medianTemp = temporaryList[2];
			

			medianList[iterations] = medianTemp;
			iterations++;
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
	/* Locate shared memory segment and attach the segment to the data space*/
	if ((shm_fd = shm_open(key, O_RDWR, 0)) < 0)
	{
		perror("shm_open");
		return 1;
	}
	if ((shared_mem = (shm*)mmap(0, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (void*)-1)
	{
		perror("mmap");
		return 1;
	}

	/* Create a thread for each level */
	pthread_t threads[num_levels];

	int level[num_levels];
	for (int i = 0; i < num_levels; i++) {

		level[i] = i;
		pthread_create(&threads[i], NULL, tempmonitor, &level[i]);
	}

	while(shared_mem->levels[thread].tempSen1 != NULL) {
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
				for (int i = 0; i < num_levels; i++) {
					pthread_mutex_lock(&shared_mem->entrances[i].SIGN.lock);

					shared_mem->entrances[i].SIGN.display = *p;

					pthread_mutex_unlock(&shared_mem->entrances[i].SIGN.lock);
				}
				usleep(20000);
			}
			alarm_active = false;
		}
		usleep(1000);
	}

	munmap(shared_mem, 2920);
	close(shm_fd);
}
