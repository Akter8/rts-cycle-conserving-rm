#include <stdio.h>

#include "configuration.h"
#include "utility.h"
#include "task.h"
#include "job.h"
#include "freq_and_voltage.h"
#include "scheduler.h"

FILE *input_tasks_file;
FILE *input_freq_file;
FILE *output_file;

int num_tasks;
Task *tasks;

long hyperperiod;
long first_in_phase_time;
long end_of_execution_time;

int num_freq_levels;
Freq_and_voltage *freq_and_voltage;
Freq_and_voltage static_freq_and_voltage;
int static_freq_and_voltage_index;

int num_freq_calculations;
int num_freq_changes;

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
    input_freq_and_voltage();

    // Sort the frequencies.
    sort_freq_and_voltage();

    // Print the frequencies.
    print_freq_and_voltage();

    // Find the static frequency and voltage.
    find_static_freq_and_voltage();

    // Create task instances.
    create_jobs();

    // Sort jobs.
    sort_jobs();

    // Print the job information.
    print_jobs();

    // Start executing task set.
    start_scheduler();

    // Print job statistics.
    capture_and_print_task_statistics();

    // Close input and output files.
    close_files();

    // Free task-set.
    delete_freq_and_voltage();
    delete_jobs();
    delete_tasks();
    
    return 0;
}
