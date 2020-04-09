#include <stdio.h>

#include "configuration.h"
#include "task.h"

FILE *input_file;
FILE *output_file;

int num_tasks;
Task *tasks;

int main(int argc, char const *argv[])
{
    // Opening the input and output files.
    input_file = fopen(INPUT_FILE_NAME, "r");
    output_file = fopen(OUTPUT_FILE_NAME, "w");

    // Read number of tasks from the input file.
    create_tasks();

    // Read the task-set.
    input_tasks();

    // Sort the task-set.
    sort_tasks();

    // Print the task-set.
    print_tasks();

    // Call scheduler.

    // Close input and output files.
    fprintf(output_file, "\n\n--------------------------- THE END ---------------------------\n");
    fclose(input_file);
    fclose(output_file);

    // Free task-set.
    delete_tasks();
    
    return 0;
}
