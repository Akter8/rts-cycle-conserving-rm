#include <stdio.h>
#include <stdlib.h>

#include "freq_and_voltage.h"
#include "utility.h"

extern FILE *input_freq_file;
extern int num_freq_levels;
extern Freq_and_voltage *freq_and_voltage;

extern FILE *output_file;

extern Freq_and_voltage static_freq_and_voltage;
extern int static_freq_and_voltage_index;


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

        if (freq_and_voltage[i].freq <= 0 || freq_and_voltage[i].freq > 1 || freq_and_voltage[i].voltage < 0)
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
    Freq_and_voltage freq_a, freq_b;
    freq_a = *(Freq_and_voltage *) a;
    freq_b = *(Freq_and_voltage *) b;

    return (freq_a.freq > freq_b.freq) - (freq_a.freq < freq_b.freq);
}


/*
 * Pre-condition: An unsorted array containing frequencies.
 * Post-condition: A sorted array containing frequencies.
 */
void
sort_freq_and_voltage()
{
    qsort (freq_and_voltage, num_freq_levels, sizeof(float), sort_freq_and_voltage_comparator);

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
    static_freq_and_voltage_index = 0;
    float task_utilisation = find_task_utilisation();

    // If the CPU utilisation of the task-set is greater than the max frequency.
    if (task_utilisation >= freq_and_voltage[num_freq_levels -1].freq)
    {
        static_freq_and_voltage_index = num_freq_levels - 1;
        static_freq_and_voltage.freq = freq_and_voltage[static_freq_and_voltage_index].freq;
        static_freq_and_voltage.voltage = freq_and_voltage[static_freq_and_voltage_index].voltage;

        fprintf(output_file, "This task-set demands static freq: %0.2f, static voltage: %0.2f\n", static_freq_and_voltage.freq, static_freq_and_voltage.voltage);

        return;
    }

    // Finding the highest possible frequency that fits the task-set.
    // Iterating through every frequency from lowest to highest.
    for (int i = 0; i < num_freq_levels; i++)
    {
        if (task_utilisation >= freq_and_voltage[i].freq)
        {
            static_freq_and_voltage_index = i;
        }
        else
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