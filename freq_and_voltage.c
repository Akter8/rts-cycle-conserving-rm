#include <stdio.h>
#include <stdlib.h>

#include "freq_and_voltage.h"
#include "utility.h"
#include "task.h"

extern FILE *input_freq_file;
extern int num_freq_levels;
extern Freq_and_voltage *freq_and_voltage;

extern FILE *output_file;

extern Freq_and_voltage static_freq_and_voltage;
extern int static_freq_and_voltage_index;

extern int num_tasks;
extern Task *tasks;


/*
 * Pre-condition: The variables to store the input frequency and voltage data.
 * Post-condition: Initialises, sorts and prints the frequency and voltage data. Also, finds the static frequency and voltage for the task-set.
 */
void
input_sort_print_freq_and_voltage()
{
    // Input the frequencies.
    input_freq_and_voltage();

    // Sort the frequencies.
    sort_freq_and_voltage();

    // Print the frequencies.
    print_freq_and_voltage();

    // Find the static frequency and voltage.
    find_static_freq_and_voltage();

    return;
}


/*
 * Pre-condition: An unitialised frequency array variable and an uninitialised variable to hold the number of frequencies.
 * Post-condition: An initialised number of frequencies variable and an initialised array containing the frequencies. 
 */
void
input_freq_and_voltage()
{
    fscanf(input_freq_file, "%d", &num_freq_levels);

    freq_and_voltage = (Freq_and_voltage *) malloc(sizeof(Freq_and_voltage) * num_freq_levels);

    for (int i = 0; i < num_freq_levels; i++)
    {
        fscanf(input_freq_file, "%f %f", &freq_and_voltage[i].freq, &freq_and_voltage[i].voltage);

        // Checking for invalid input. Frequency cannot be non-positive and greater than 1. Voltage cannot be non-positive.
        if (freq_and_voltage[i].freq <= 0 || freq_and_voltage[i].freq > 1 || freq_and_voltage[i].voltage <= 0)
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
sort_freq_and_voltage_comparator(const void *a, const void *b)
{
    float freq_a, freq_b;
    freq_a = (*(Freq_and_voltage *) a).freq;
    freq_b = (*(Freq_and_voltage *) b).freq;

    return (freq_a >= freq_b) ? 1 : -1;
}


/*
 * Pre-condition: An unsorted array containing frequencies.
 * Post-condition: A sorted array containing frequencies.
 */
void
sort_freq_and_voltage()
{
    qsort (freq_and_voltage, num_freq_levels, sizeof(Freq_and_voltage), sort_freq_and_voltage_comparator);

    return;
}


/*
 * Pre-condition: The arrays containing the values of freq and voltages.
 * Post-condition: Prints the freq and voltage values onto the output file.
 */
void
print_freq_and_voltage()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Frequencies and voltages available (relative to Fmax).\n");
    fprintf(output_file, "Number of levels of frequencies and voltages: %d\n", num_freq_levels);
    for (int i = 0; i < num_freq_levels; i++)
    {
        fprintf(output_file, "%0.2f (%0.2fV), ", freq_and_voltage[i].freq, freq_and_voltage[i].voltage);
    }
    fprintf(output_file, "\n");

    return;
}


/*
 * Pre-condition: The valid set of frequencies and corresponing voltages.
 * Post-condition: Deallocates the heap data that was allocated to store the frequencies and voltages.
 */
void
delete_freq_and_voltage()
{
    free(freq_and_voltage);

    return;
}


/*
 * Pre-condition: The valid task-set and valid set of frequencies and corresponing voltages.
 * Post-condition: The static voltage and frequency for this task-set.
 */
void
find_static_freq_and_voltage()
{
    // Initialisation.
    static_freq_and_voltage_index = num_freq_levels - 1;

    // Finding the highest possible frequency that fits the task-set.
    // Iterating through every frequency from highest to lowest.
    for (int i = num_freq_levels - 1; i >= 0; i--)
    {
        if (rm_test(freq_and_voltage[i].freq) == 1) // If 1, then this freq fits.
        {
            static_freq_and_voltage_index = i;
        }
        else // Else break and take the previous level that fit this task set.
        {
            break;
        }
        
    }

    // Updating the static freq and voltage.
    static_freq_and_voltage.freq = freq_and_voltage[static_freq_and_voltage_index].freq;
    static_freq_and_voltage.voltage = freq_and_voltage[static_freq_and_voltage_index].voltage;
    fprintf(output_file, "This task-set demands static freq: %0.2f, static voltage: %0.2f\n", static_freq_and_voltage.freq, static_freq_and_voltage.voltage);

    return;
}


/*
 * Pre-condition: The task set information about periods and worst case execution time. The relative frequency at some level that is to be tested.
 * Post-condition: Returns a value indicating the result of the test (1 = pass and 0 = fail).
 */
int
rm_test(float relative_freq)
{
    /*
     * For all tasks Ti and Periods sorted in ascending order,
     *      if (floor(Pi/P1) * C1) + (floor(Pi/P2) * C2) + ......... + (floor(Pi/Pi) * Ci) <= relative_freq * Pi
     *      then return 1
     *      else 0
     */
    for (int i = 0; i < num_tasks; i++) // Iterating through all tasks.
    {
        float sum = 0;
        for (int k = 0; k <= i; k++) // Iterating till the ith period.
        {
            int int_divide = tasks[i].period / tasks[k].period;
            sum += int_divide * tasks[k].wcet;
        }

        if (sum > tasks[i].period * relative_freq)
        {
            return 0;
        }
    }

    return 1;
}