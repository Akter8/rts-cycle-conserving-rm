#include <stdio.h>

#include "configuration.h"
#include "utility.h"
#include "task.h"
#include "job.h"
#include "freq_and_voltage.h"
#include "scheduler.h"

// I/O Files.
FILE *input_tasks_file;
FILE *input_freq_file;
FILE *output_file;
FILE *statistics_file;

// Variables to hold the task and job data.
int num_tasks;
Task *tasks;

int num_jobs;
Job *jobs;

// Timing parameters of the program..
long hyperperiod;
long first_in_phase_time;
long end_of_execution_time;

// Global variables related to the DVFS part of the program.
int num_freq_levels;
Freq_and_voltage *freq_and_voltage;
Freq_and_voltage static_freq_and_voltage;
int static_freq_and_voltage_index;

long num_freq_calculations;
long num_freq_changes;
float total_dynamic_energy;


/*
 * Pre-condition: The relevant data in the input files (tasks and frequency inputs).
 * Post-condition: Runs the program from the start to the end.
 */
int main(int argc, char const *argv[])
{
    /*
     * Opens the input and output files. 
     * Initialises, sorts and prints the data.
     * Finds the static voltage and frequency for the task set.
     * Finds the hyperperiod.
     * Finds the first in-phase time and time till which the scheduler has to schedule.
     */
    open_files_and_init_data();

    /* Starts the scheduler and then schedules the entire task-set. */
    start_scheduler();

    /*
     * Takes care of things to be done when exiting from the program.
     * Closes the input and output files.
     * Deallocates the data from the heap.
     */
    close_files_and_delete_data();
    
    return 0;
}
