#include <stdio.h>

#include "configuration.h"
#include "utility.h"
#include "task.h"

FILE *input_tasks_file;
FILE *input_freq_file;
FILE *output_file;

int num_tasks;
Task *tasks;
float hyperperiod;

int num_freq;
float *freq;

int main(int argc, char const *argv[])
{
    // Opening the input and output files.
    input_tasks_file = fopen(INPUT_TASKS_FILE_NAME, "r");
    input_freq_file = fopen(INPUT_FREQ_FILE_NAME, "r");
    output_file = fopen(OUTPUT_FILE_NAME, "w");

    // Read number of tasks from the input file.
    create_tasks();

    // Read the task-set.
    input_tasks();

    // Sort the task-set.
    sort_tasks();

    // Print the task-set.
    print_tasks();

    // Find the hyperperiod.
    hyperperiod = find_hyperperiod();
    fprintf(output_file, "\n\nHyperperiod: %0.1f\n", hyperperiod);

    // Input the frequencies.
    input_freq();

    // Sort the frequencies.
    sort_freq();

    // Print the frequencies.
    print_freq();

    // Call scheduler.

    // Close input and output files.
    fprintf(output_file, "\n\n--------------------------- THE END ---------------------------\n");
    fclose(input_tasks_file);
    fclose(input_freq_file);
    fclose(output_file);

    // Free task-set.
    delete_tasks();
    
    return 0;
}
