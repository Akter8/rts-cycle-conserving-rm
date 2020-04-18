# RTS-Assignment-3

* Group number: 1
* Topic: Cycle Conserving RM

## Contributors

* Akhil Tarikere, 2017A7PS1916G
* Ashish Patel, 2019H1030028G
* Garima Mangal, 2019H1030561G
* Ratti Sai Pavan, 2019H1030505G

## Doubts

## Tasks left

* [ ] Convert README.md to README.txt before submission.
* [ ] Rename the tar file of the program to the group number.

## Description of Files

* configuration.h - Contains the configurable inputs to the program.
* driver.c - Contains the driver function of the program.
* task.h - Contains the definition and function declaration of the Task ADT.
* task.c - Contains the implementation of the Task ADT functions.
* job.h - Contains the ADT of a task instance.
* job.c - Contains the function implementation of Jobs or task instances.
* freq_and_voltage.h - Contains the struct of the ADT and the function declarations related to frequency and voltage inputs.
* freq_and_voltage.c - Contains the function implementations related to frequency and voltage inputs.
* utility.h - Contains function declarations of utility functions.
* utility.c - Contains the function implementation of utiility functions.
* scheduler.h - Contains the struct definitions and functions definitions required for scheduling the task-set.
* scheduler.c - Contains the implementation of the dynamic scheduler.
* Makefile - Contains the compilation commands of the program.

### Input files

* input_tasks_file.txt - Contains the task inputs.
* input_freq_file.txt - Contains the frequency inputs.

### Output files (Only after running the program)

* output_file.txt - Contains the output of the program.
* output_statistics_file.txt - Contains the output statistics of the program.

## How to compile and run

* Make the required changes in configuration.h
* Make the required changes in the input files.
* Run "make" on the terminal (in the directory of the program) to compile the program.
* Run the executable defined in Makefile to run the program.
* Run "make clean" to remove the compilation from the working directory.

## Description of the structure of the program

* First the program inputs the required data-- task-set information and freq (and voltage) information (at the same time it checks if the data is valid).
* Then the program sorts all the tasks and freq as required.
* Then the program creates instances of tasks till the required time.
* Then the program finds the static frequency and voltage for the task-set.
* Then the program creates various data-structures related to the scheduler.
* Using the data-structures previously defined, the program simulates the dynamic scheduling of the task-set using the given frequency (and voltage) input, also while dynamically changing the frequency and voltage at decision points of the schedule.
* Then the program outputs the schedule and the various statistics of execution of scheduler.

## Assumptions and limitations

* Frequency-calculation overhead time was given as 0.05 units, but there was nothing about frequency-change overhead time. So we have assumed a frequency-change time as 0.1 units of time (double of frequency-calculation overhead). (See configuration.h)
* The phase, period and deadline parameters of a task can only be integers. This is because integers do not cause a loss of accuracy and integers are much easier to work with in terms of schedulers with non-integer granularities.
* The actual execution time essentially calculated and known before hand in this simulation so as to calculate the next decision point. Otherwise it is not possible to simulate a schedule by only calling the scheduler at decision points.
* In the CC-RM algorithm originally devised by Pillai et al, the allocate_cycles() function is only called when a new job arrives. In this program, both allocate_cycles and select_frequency() are called at every decision point as jobs run from 50%-100% of their wcet and the actual execution time is not known by the allocate_cycles().
* Task-sets with worst-case task utilisation > 1 might or might not be schedulable because psuedo random numbers are being used to find the actual execution time. So the simulation still runs for task-sets with worst-case task utilisation > 1, only that it might not finish scheduling.
* The program contains lots of global variables being used to transfer data between functions. Not a problem at this scale of programming, but would be a problem if and when the program would scale up.
