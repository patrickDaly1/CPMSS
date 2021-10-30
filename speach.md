
* Note all parts of assignemnt that are wanting to be marked need to be demonstrated in the video presentation 
# Manager

## Simulator
## Setup in main
    - sim begins by setting up the predefined shared memory structure
    - then moves on to initialise all the mutex and cond variables
    - sim then creates threads for a series of functions which will be covered later
    - currently no proper way to kill the program as it was not outlined as a required specification

## Car generation
    - car init is a function responsible for generating unique cars
    - begins by assigning car to a random enterance 
    - there is the a 50/50 chance that cars are generaged with a random lp or one from plates.txt
    - the car is then checked to ensure it is not currently within the system

    - car init calls this function and then adds cars to linked list representing entry queue

## Boom gates
    - entry boom gates are responsible for letting cars into carpark
    - boom gate finds first car in the entry queue at the relitive enterence
    - lp is then passed to manager through lpr reader and checks if car is allowed in
    - if not allowed in car is removed from sim, if allowed in the sim waits for signals from the manager to open the boom gate and preforms simulation of booms opening
    - when boom is open sim creates a new thread for the car allowed in and removes it from the entry queue and adds it to the in carpark queue

    - exit boom preforms similar process
    - boom checks for first car at exit and passes lp to lpr reader
    - boom then wait form open and close signal from manager and performs simulation of this process
    - during close boom removes car from in exit queue 

## Car behaviour
    - car begins by traveling to assigned level where it triggers lpr upon entry
    - car then parks for random amount of time
    - car then generates random exit to go to
    - car triggers lpr and begins traveling to exit where upon arrival car is removed from in carpark linked list and added to exit queue

## Temp sensors
    - generates random values which are passed to the temp sensors to be read by fire alarm system
    - functionality for triggering fire after random period of time

# Fire alarem