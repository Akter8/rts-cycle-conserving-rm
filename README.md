# RTS-Assignment-3: Cycle Conserving RM

## Contributors

* Akhil Tarikere, 2017A7PS1916G
* Ashish Patel, 2019H1030028G
* Garima Mangal, 2019H1030561G
* Ratti Sai Pavan, 2019H1030505G

## Doubts

## Tasks left

* [X] Added a new separate module for frequency and voltage.
* [X] Find static voltage and frequency.
* [ ] Define the relevant data-structures for the scheduler.
* [ ] Schedule the task-set dynamically for CC-RM.
* [ ] Write the reason why phase, period and deadlines are integers.
* [ ] As of now, everything works till hyperperiod, make it work till min(3 * hyperperiod, first in-phase time + hyperperiod).
* [ ] Make the driver module and function to have only 20 lines of code at max.
* [ ] Convert README.md to README.txt before submission.
* [ ] Rename the tar file of the program to the group number.

## Things to bear in mind

* Call the scheduler only when there is a decision point-- arrival or termination.
* Schedule till min(3 * hyperperiod, first in-phase time + hyperperiod).
* Only periodic jobs execute. They execute for 50-100% of the wcet.
* Implicit deadlines for jobs.
* Output: response times (min, max, avg), response time jitters (relative and absolute), execution times (min, max, avg), waiting times (min, max, avg).

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

### Output files

* output_file.txt - Contains the output of the program.

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
* Then the program creates various data-structures related to the scheduler.
* Using the data-structures previously defined, the program simulates the dynamic scheduling of the task-set using the given frequency (and voltage) input.
* Then the program outputs the various statistics of execution of tasks.

## Assumptions
