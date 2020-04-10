#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "utility.h"
#include "configuration.h"
#include "task.h"

extern int num_tasks;
extern FILE *input_tasks_file;
extern Task *tasks;

extern FILE *input_freq_file;
extern int num_freq;
extern float *freq;

extern float hyperperiod;
float first_in_phase_time;
extern float end_of_execution_time;

extern FILE *output_file;


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
    fclose(input_tasks_file);
    fclose(input_freq_file);
    fclose(output_file);

    return;
}


/*
 * Pre-condition: Newly opened files.
 * Post-condition: Error if file open resulted in error.
 */
void
file_not_null_check()
{
    if (!input_tasks_file || !input_freq_file || !output_file)
    {
        fprintf(stderr, "ERROR: Could not open the required files.\n");
        exit(0);
    }

    return;
}


/*
 * Pre-condition: An unitialised frequency array variable and an uninitialised variable to hold the number of frequencies.
 * Post-condition: An initialised number of frequencies variable and an initialised array containing the frequencies. 
 */
void
input_freq()
{
    fscanf(input_freq_file, "%d", &num_freq);

    freq = (float *) malloc(sizeof(float) * num_freq);

    for (int i = 0; i < num_freq; i++)
    {
        fscanf(input_freq_file, "%f", &freq[i]);

        if (freq[i] <= 0 || freq[i] > 1)
        {
            fprintf(stderr, "ERROR: Invalid input in frequency input file. Please enter valid data\n");
            exit(0);
        }

    }
    

    return;
}


/*
 * Pre-condition: Two frequency instances.
 * Post-condition: An integer value of the comparision between the two frequency instances.
 */
int
sort_freq_comparator(const void *a, const void *b)
{
    float freq_a, freq_b;
    freq_a = *(float *) a;
    freq_b = *(float *) b;

    return (freq_a > freq_b) - (freq_a < freq_b);
}


/*
 * Pre-condition: An unsorted array containing frequencies.
 * Post-condition: A sorted array containing frequencies.
 */
void
sort_freq()
{
    qsort (freq, num_freq, sizeof(float), sort_freq_comparator);

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
print_freq()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Frequencies available (relative to Fmax).\n");
    fprintf(output_file, "Number of frequencies: %d\n", num_freq);
    for (int i = 0; i < num_freq; i++)
    {
        fprintf(output_file, "%0.2f, ", freq[i]);
    }
    fprintf(output_file, "\n");

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
    float ans = 0;
    // for (int i = 0; i < hyperperiod; i++)
    // {
        
    // }

    first_in_phase_time = ans;

    return;
}


/*
 * Pre-condition: An array containing valid task-set data.
 * Post-condition: A number corresponding to the the end of execution time.
 */
void
find_end_of_execution_time()
{
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
    find_hyperperiod();
    find_first_in_phase_time();
    find_end_of_execution_time();

    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Hyperperiod: %0.2f\n", hyperperiod);
    fprintf(output_file, "First in-phase time: %0.2f\n", first_in_phase_time);
    fprintf(output_file, "End of execution time: %0.2f\n", end_of_execution_time);

    for (int i = 0; i < num_tasks; i++)
    {
        tasks[i].num_instances = ((int) end_of_execution_time) / tasks[i].period; 
    }

    return;
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