#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utility.h"
#include "configuration.h"
#include "task.h"
#include "freq_and_voltage.h"
#include "job.h"

extern int num_tasks;
extern Task *tasks;

extern long hyperperiod;
extern long first_in_phase_time;
extern long end_of_execution_time;

extern FILE *input_tasks_file;
extern FILE *input_freq_file;
extern FILE *output_file;
extern FILE *statistics_file;


/*
 * Pre-condition: Uninitialised I/O file pointers.
 * Post-condition: Initialises I/O file pointers. Creates, sorts and prints the tasks, freq and voltage, jobs. Also finds the static frequency and voltage for the task-set.
 */
void
open_files_and_init_data()
{
    // Opening the I/O files.
    input_tasks_file = fopen(INPUT_TASKS_FILE_NAME, "r");
    input_freq_file = fopen(INPUT_FREQ_FILE_NAME, "r");
    output_file = fopen(OUTPUT_FILE_NAME, "w");
    statistics_file = fopen(OUTPUT_STATISTICS_FILE_NAME, "w");

    // Checking for errors in file opening.
    files_not_null_check();

    // Create, input, sort and print the task-set.
    create_input_sort_print_tasks();

    // Input, sort and print the frequency and voltage inputs. Also finds the static frequency and voltage.
    input_sort_print_freq_and_voltage();

    // Create, sort and print jobs.
    create_sort_print_jobs();

    return;
}


/*
 * Pre-condition: Open I/O files after execution of the program.
 * Post-condition: Closes the given I/O files.
 */
void
close_files_and_delete_data()
{
    // Closing files.
    fprintf(output_file, "\n--------------------------- THE END ---------------------------\n");
    fprintf(statistics_file, "\n--------------------------- THE END ---------------------------\n");
    fclose(input_tasks_file);
    fclose(input_freq_file);
    fclose(output_file);
    fclose(statistics_file);

    // Free task-set.
    delete_freq_and_voltage();
    delete_jobs();
    delete_tasks();

    return;
}


/*
 * Pre-condition: Newly opened files.
 * Post-condition: Error if file open resulted in error.
 */
void
files_not_null_check()
{
    // File pointer is null when there is an error in opening the files.
    if (!input_tasks_file || !input_freq_file || !output_file || !statistics_file)
    {
        fprintf(stderr, "ERROR: Could not open the required files.\n");
        exit(0);
    }

    return;
}


/*
 * Pre-condition: Two numbers.
 * Post-condition: The GCD of the two given numbers.
 */
float
gcd(float a, float b)
{
    if (b == 0) {
        return a;
    }

    return gcd(b, fmod(a,b));
}


/*
 * Pre-condition: Two numbers.
 * Post-condition: The LCM of the two given numbers.
 */
float 
lcm(float a, float b) {
    return (a * b) / gcd(a,b);
}


/*
 * Pre-condition: Valid task-set data containing the periods of the tasks.
 * Post-condition: The hyperperiod of the given task-set.
 */
void
find_hyperperiod()
{
    // Hyperperiod = lcm of all task's period in the task-set.

    int current_lcm = 1;

    for (int i = 0; i < num_tasks; i++) // Iterating through each task in the task-set.
    {
        current_lcm = lcm(current_lcm, tasks[i].period);
    }

    hyperperiod = current_lcm;
}


/*
 * Pre-condition: An array containing valid task-set data.
 * Post-condition: A number corresponding to the the first in phase time of the task set.
 */
void
find_first_in_phase_time()
{
    first_in_phase_time = -1;

    long max_phase = find_max_phase();
    
    // Finding if the first in-phase time is within the time = 2*hyperperiod. If not we take first_in_phase_time = -1.
    for (long i = max_phase; i <= 2 * hyperperiod; i++)
    {
        bool success = true;
        for (long j = 0; j < num_tasks;  j++) // Iterating through each task.
        {
            if ((i - tasks[j].phase) % (tasks[j].period) != 0) // Even if one task does not arrive at time = i, then i cannot be the first in-phase time. 
            {
                success = false;
                break;
            }
        }
        if (success)
        {
            first_in_phase_time = i;
            return;
        }
    }

    // If control reaches here, then the fist in-phase time is not withiin (0, 2 * hyperperiod).
    fprintf(output_file, "Could not find the first in-phase time within (0, 2 * hyperperiod).\n");

    return;
}


/*
 * Pre-condition: An array containing valid task-set data.
 * Post-condition: A number corresponding to the the end of execution time.
 */
void
find_end_of_execution_time()
{
    if (first_in_phase_time  <= 2 * hyperperiod)
    {
        fprintf(output_file, "First in-phase time < 2 * hyperperiod. Scheduling till first in-phase time + hyperperiod.\n");
        end_of_execution_time = first_in_phase_time + hyperperiod;
    }
    else
    {
        fprintf(output_file, "First in-phase time > 2 * hyperperiod. Scheduling till 3 * hyperperiod\n");
        end_of_execution_time = 3 * hyperperiod;
    }

    return;
}


/*
 * Pre-condition: The task-set data containing the periods of the tasks.
 * Post-condition: The number of instances for every task in the given time frame.
 */
void
calculate_num_instances_of_tasks()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    find_hyperperiod();
    find_first_in_phase_time();

    // Find the worst-case task utilisation of the task-set.
    find_task_utilisation();

    fprintf(output_file, "Hyperperiod: %ld\n", hyperperiod);
    fprintf(output_file, "First in-phase time: %ld\n", first_in_phase_time);

    // End of execution time is min(3*hyperperiod, first in-phase time + hyperperiod).
    find_end_of_execution_time();
    fprintf(output_file, "End of execution time: %ld\n", end_of_execution_time);

    for (int i = 0; i < num_tasks; i++) // Iterating through each task in the task-set.
    {
        tasks[i].num_instances = ((int) end_of_execution_time) / tasks[i].period; // Floor function automatically happens in integer division.
    }

    return;
}

/*
 * Pre-condition: The periods and wcet of all tasks.
 * Post-condition: Finds the worst-case CPU utilisation value for the given task-set.
 */
float
find_task_utilisation()
{
    float task_utilisation = 0; // To calculate the worst-case CPU utilisation.
    for (int i = 0; i < num_tasks; i++) // Iterating through each task in the task-set.
    {
        task_utilisation += (tasks[i].wcet / tasks[i].period);
    }

    if (task_utilisation > 1)
    {
        fprintf(output_file, "Task set has a utilisation: %0.2f > 1. Might NOT be able to schedule all jobs completely.\n", task_utilisation);
    }
    else
    {
        fprintf(output_file, "Task set has a utilisation: %0.2f <= 1. Might be able to schedule all jobs completely.\n", task_utilisation);
    }
    
    return task_utilisation;
}


/*
 * Pre-condition: A float value.
 * Post-condition: The absolute value of the given float number.
 */
float
floatAbs(float a)
{
    return (a > 0 ? a : -a);
}