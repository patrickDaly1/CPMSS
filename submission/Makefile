#Makefile

all: simulator manager firealarm

simulator:
    gcc -Wall -pedantic simulator.c -o simulator linkedlist.h -lrt -pthread

manager:
    gcc -Wall -pedantic manager.c -o manager -lrt -pthread

firealarm:
    gcc -Wall -pedantic firealarm.c -o firealarm -lrt -pthread