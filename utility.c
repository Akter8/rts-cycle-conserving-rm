#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utility.h"
#include "configuration.h"
#include "task.h"

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
 * Post-condition: Initialised I/O file pointers.
 */
void
open_files()
{
    input_tasks_file = fopen(INPUT_TASKS_FILE_NAME, "r");
    input_freq_file = fopen(INPUT_FREQ_FILE_NAME, "r");
    output_file = fopen(OUTPUT_FILE_NAME, "w");
    statistics_file = fopen(OUTPUT_STATISTICS_FILE_NAME, "w");

    // Checking for errors in file opening.
    file_not_null_check();

    return;
}


/*
 * Pre-condition: Open I/O files after execution of the program.
 * Post-condition: Closes the given I/O files.
 */
void
close_files()
{
    fprintf(output_file, "\n\n--------------------------- THE END ---------------------------\n");
    fprintf(statistics_file, "\n\n--------------------------- THE END ---------------------------\n");
    fclose(input_tasks_file);
    fclose(input_freq_file);
    fclose(output_file);
    fclose(statistics_file);

    return;
}


/*
 * Pre-condition: Newly opened files.
 * Post-condition: Error if file open resulted in error.
 */
void
file_not_null_check()
{
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
    int current_lcm = 1;

    for (int i = 0; i < num_tasks; i++)
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
    
    for (long i = max_phase; i < 2 * hyperperiod; i++)
    {
        bool success = true;
        for (long j = 0; j < num_tasks;  j++)
        {
            if ((i - tasks[j].phase) % (tasks[j].period) != 0)
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
    if (first_in_phase_time + hyperperiod <= 3 * hyperperiod)
    {
        fprintf(output_file, "First in-phase time < 2 * hyperperiod. Scheduling till first in-phase time + hyperperiod.\n");
        end_of_execution_time = first_in_phase_time + hyperperiod;
    }
    else
    {
        fprintf(output_file, "First in-phase time > 2 * hyperperiod. Scheduling till 3 * hyperperiod\n");
        end_of_execution_time = 3 * hyperperiod;
    }
    
    end_of_execution_time = fmin(3 * hyperperiod, first_in_phase_time + hyperperiod);

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

    fprintf(output_file, "Hyperperiod: %ld\n", hyperperiod);
    fprintf(output_file, "First in-phase time: %ld\n", first_in_phase_time);

    // End of execution time is min(3*hyperperiod, first in-phase time + hyperperiod).
    find_end_of_execution_time();
    fprintf(output_file, "End of execution time: %ld\n", end_of_execution_time);

    for (int i = 0; i < num_tasks; i++)
    {
        tasks[i].num_instances = ((int) end_of_execution_time) / tasks[i].period; 
    }

    return;
}

/*
 * Pre-condition: The periods and wcet of all tasks.
 * Post-condition: The CPU utilisation value for the given task-set.
 */
float
find_task_utilisation()
{
    float task_utilisation = 0;
    for (int i = 0; i < num_tasks; i++)
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