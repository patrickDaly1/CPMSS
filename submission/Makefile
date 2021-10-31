#Makefile

all: simulator manager

simulator:
	gcc -Wall -pedantic simulator.c -o simulator linkedlist.h hashtable.h -lrt -pthread

manager:
	gcc -Wall -pedantic manager.c -o manager -lrt -pthread