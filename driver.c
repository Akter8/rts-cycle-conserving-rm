#include <stdio.h>

#include "configuration.h"
#include "utility.h"
#include "task.h"
#include "job.h"

FILE *input_tasks_file;
FILE *input_freq_file;
FILE *output_file;

int num_tasks;
Task *tasks;

float hyperperiod;
float first_in_phase_time;
float end_of_execution_time;

int num_freq;
float *freq;

int num_jobs;
Job *jobs;

int main(int argc, char const *argv[])
{
    // Opening the input and output files.
    open_files();

    // Read number of tasks from the input file.
    create_tasks();

    // Read the task-set.
    input_tasks();

    // Sort the task-set.
    sort_tasks();

    // Print the task-set.
    print_tasks();

    // Input the frequencies.
    input_freq();

    // Sort the frequencies.
    sort_freq();

    // Print the frequencies.
    print_freq();

    // Create task instances.
    create_jobs();

    // Sort jobs.
    sort_jobs();

    // Print the job information.
    print_jobs();

    // Call scheduler.

    // Print job statistics.
    capture_and_print_task_statistics();

    // Close input and output files.
    close_files();

    fprintf(output_file, "abcd\n");
    // Free task-set.
    delete_tasks();
    
    return 0;
}
