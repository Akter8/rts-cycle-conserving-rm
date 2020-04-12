#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "configuration.h"
#include "utility.h"
#include "task.h"
#include "job.h"
#include "freq_and_voltage.h"

extern FILE *input_tasks_file;
extern FILE *output_file;
extern FILE *statistics_file;

extern int num_tasks;
extern Task *tasks;

extern int num_jobs;
extern Job *jobs;

extern Freq_and_voltage *freq_and_voltage;


/*
 * Pre-condition: An uninitialised variable num_tasks, uninitialised variable to hold the task set information.
 * Post-condition: Initialise num_tasks, create space and initialise the heap memory to store task data. Sorts the task-set and then prints it.
 * 
 * This function acts as a init_tasks() function and calls all the other initialising functions.
 */
void
create_input_sort_print_tasks()
{
    // Read number of tasks from the input file.
    create_tasks();

    // Read the task-set.
    input_tasks();

    // Sort the task-set.
    sort_tasks();

    // Print the task-set.
    print_tasks();

    return;
}


/*
 * Pre-condition: An uninitialised variable num_tasks.
 * Post-condition: Initialise num_tasks and create space in the heap to store task data.
 * 
 * This function creates space for task data.
 */
void
create_tasks()
{
    fscanf(input_tasks_file, "%d", &num_tasks);

    tasks = (Task *) malloc(sizeof(Task) * num_tasks);

    return;
}


/*
 * Pre-condition: An unitialised malloc array of tasks.
 * Post-condition: Initialised array of tasks.
 * 
 * This function reads the input file and initialises the data.
 */
void
input_tasks()
{
    for (int i = 0; i < num_tasks; i++) // Iterates over all tasks.
    {
        tasks[i].task_num = i;
        fscanf(input_tasks_file, "%ld %ld %f %ld", &tasks[i].phase, &tasks[i].period, &tasks[i].wcet, &tasks[i].deadline);

        // Checks if the data inputted is valid or not.
        if ((tasks[i].period < tasks[i].wcet) || (tasks[i].phase < 0 || tasks[i].period < 0 || tasks[i].wcet < 0 || tasks[i].deadline < 0))
        {
            fprintf(stderr, "ERROR: Invalid task input. Please input valid data.\n");
            exit(0);
        }
    }

    // Finding the number of instances for every task.
    calculate_num_instances_of_tasks();

    return;
}


/*
 * Pre-condition: Task data.
 * Post-condition: Nothing.
 * 
 * Prints a human readable version of task data onto the output file.
 */
void
print_tasks()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Task-set Information (sorted based on period).\n");
    fprintf(output_file, "Number of tasks: %d\n", num_tasks);

    for (int i = 0; i < num_tasks; i++) // Iterates over each task.
    {
        Task task = tasks[i];
        fprintf(output_file, "Task-%d: Phase: %ld, Period: %ld, WCET: %0.1f, Deadline: %ld, Number of instances: %d\n", task.task_num, task.phase, task.period, task.wcet, task.deadline, task.num_instances);
    }

    return;
}


/*
 * Pre-condition: Two different tasks.
 * Post-condition: Integer representing the answer of the comparision.
 * 
 * Compares the two tasks based on period. If the periods are the same, then it compares based on the wcet.
 */
int
sort_tasks_comparator(const void *a, const void *b)
{
    Task task_a, task_b;
    task_a = *((Task *) a);
    task_b = *((Task *) b);

    if (task_a.period != task_b.period) // If the tasks can be compared on their periods.
    {
        return task_a.period - task_b.period;
    }
    else // Else compare based on their wcet.
    {
        return task_a.wcet - task_b.wcet;
    }
    
}


/*
 * Pre-condition: sorted or unsorted tasks data.
 * Post-condition: Task data in a sorted manner.
 * 
 * Sorts all tasks based on the comparator function.
 */
void
sort_tasks()
{
    qsort((void *) tasks, num_tasks, sizeof(tasks[0]), sort_tasks_comparator);

    return;
}


/*
 * Pre-condition: Task data containing the valid response times after execution.
 * Post-condition: Calculates the min, max and avg response times.
 * 
 * Prints the response time statistics for all the tasks and all their instances.
 */
void
print_response_times()
{
    fprintf(statistics_file, "\nResponse time statistics:\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through each task.
    {
        float max = -FLT_MAX, min = FLT_MAX, avg = 0;
        Task task = tasks[i];
        float response_time;

        fprintf(statistics_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++) // Iterates through each instance of the task.
        {
            response_time = task.response_times[j];

            fprintf(statistics_file, "%0.1f, ", response_time);

            avg += response_time;
            if (response_time > max)
                max = response_time;
            if (response_time < min)
                min = response_time;
        }
        fprintf(statistics_file, "\n\t(Max: %0.1f, Min:%0.1f, Avg: %0.1f)\n", max, min, avg / task.num_instances);
    }

    return;
}


/*
 * Pre-condition: Task data containing respone times after execution.
 * Post-condition: Absolute and relative response time jitters for every task in the task set.
 */
void
print_response_time_jitters()
{
    fprintf(statistics_file, "\nResponse time jitter statistics.\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through each task in the task-set.
    {
        Task task = tasks[i];

        fprintf(statistics_file, "Task-%d: Absolute RTJ: ", task.task_num);
        float max = -FLT_MAX, min = FLT_MAX;
        float response_time;

        // Absolute RTJ calculation.
        for (int j = 0; j < task.num_instances; j++) // Iterates through each instance of a given task.
        {
            response_time = task.response_times[j];

            if (response_time > max)
                max = response_time;
            if (response_time < min)
                min = response_time;
        }
        fprintf(statistics_file, "%0.1f, Relative RTJ: ", max - min);

        // If the number of instances in the hyperperiod is only 1, then the relative RTJ will be 0.
        if (task.num_instances == 1)
        {
            fprintf(statistics_file, "0.0\n");
            continue;
        }

        // Relative RTJ calculation.
        max = -FLT_MAX;
        float relative_rtj;
        for (int j = 0; j < task.num_instances - 1; j++) // Iterates through each instance of a given task.
        {
            relative_rtj = floatAbs(task.response_times[j] - task.response_times[j+1]);

            if (relative_rtj > max)
                max = relative_rtj;
        }
        
        // Also have to calculate the RTJ of the last and first instance, as after the hyperperiod, the first instance will run again.
        relative_rtj = floatAbs(task.response_times[0] - task.response_times[task.num_instances - 1]);

        if (relative_rtj > max)
            max = relative_rtj;

        fprintf(statistics_file, "%0.1f\n", max);
    }

    return;
}


/*
 * Pre-condition: Task data containing the valid execution times after execution.
 * Post-condition: Calculates the min, max and avg execution times.
 * 
 * Prints the execution time statistics for all the tasks and all their instances.
 */
void
print_execution_times()
{
    fprintf(statistics_file, "\nExecution time statistics:\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through each task in the task set.
    {
        float max = -FLT_MAX, min = FLT_MAX, avg = 0;
        Task task = tasks[i];
        float execution_time;

        fprintf(statistics_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++) // Iterates through every instance of the given task.
        {
            execution_time = task.execution_times[j];

            fprintf(statistics_file, "%0.1f, ", execution_time);

            avg += execution_time;
            if (execution_time > max)
                max = execution_time;
            if (execution_time < min)
                min = execution_time;
        }
        fprintf(statistics_file, "\n\t(Max: %0.1f, Min:%0.1f, Avg: %0.1f)\n", max, min, avg / task.num_instances);
    }

    return;
}


/*
 * Pre-condition: Task data containing the valid response and execution times after execution.
 * Post-condition: Calculates the min, max and avg waiting times.
 * 
 * Prints the waiting time statistics for all the tasks and all their instances.
 */
void
print_waiting_times()
{
    fprintf(statistics_file, "\nWaiting time statistics:\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through every task in the task-set.
    {
        float max = -FLT_MAX, min = FLT_MAX, avg = 0;
        Task task = tasks[i];
        float waiting_time;

        fprintf(statistics_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++) // Iterates through every instance of the task.
        {
            // Waiting time = response time - execution time.
            waiting_time = task.response_times[j] - task.execution_times[j];

            fprintf(statistics_file, "%0.1f, ", waiting_time);

            avg += waiting_time;
            if (waiting_time > max)
                max = waiting_time;
            if (waiting_time < min)
                min = waiting_time;
        }
        fprintf(statistics_file, "\n\t(Max: %0.1f, Min:%0.1f, Avg: %0.1f)\n", max, min, avg / task.num_instances);
    }

    return;
}


/*
 * Pre-condition: Task data containing the valid frequencies after execution.
 * Post-condition: Calculates the min, max and avg frequencies of execution.
 * 
 * Prints the frequency of execution statistics for all the tasks and all their instances.
 */
void
print_execution_freqs()
{
    fprintf(statistics_file, "\nExecution frequency statistics:\n");
    fprintf(statistics_file, "Disclaimer: Frequency shown is relative to the maximum frequency. Voltage shown is in absolute values.\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through each task in the task set.
    {
        float max_freq = -FLT_MAX, min_freq = FLT_MAX, avg_freq = 0;
        float max_voltage = -FLT_MAX, min_voltage = FLT_MAX, avg_voltage = 0;
        Task task = tasks[i];
        float execution_freq;
        float execution_voltage;

        fprintf(statistics_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++) // Iterates through each task instance of the given task.
        {
            execution_freq = freq_and_voltage[task.execution_freq_indices[j]].freq;
            execution_voltage = freq_and_voltage[task.execution_freq_indices[j]].voltage;

            fprintf(statistics_file, "%0.2f (%0.2fV), ", execution_freq, execution_voltage);

            avg_freq += execution_freq;
            avg_voltage += execution_voltage;

            if (execution_freq > max_freq)
                max_freq = execution_freq;
            if (execution_freq < min_freq)
                min_freq = execution_freq;

            if (execution_voltage > max_voltage)
                max_voltage = execution_voltage;
            if (execution_voltage < min_voltage)
                min_voltage = execution_voltage;
        }
        fprintf(statistics_file, "\n\t(Max Freq: %0.1f, Min Freq:%0.1f, Avg Freq: %0.1f)\n", max_freq, min_freq, avg_freq / task.num_instances);
        fprintf(statistics_file, "\t(Max Voltage: %0.1fV, Min Voltage:%0.1fV, Avg Voltage: %0.1fV)\n", max_voltage, min_voltage, avg_voltage / task.num_instances);
    }

    return;
}


/*
 * Pre-condition: The task array containing the dynamic power consumed by every instance.
 * Post-condition: Prints the dynamic energy consumed by every instance. Also finds the min, max and avg of the same.
 */
void
print_dynamic_energy_consumed()
{
    fprintf(statistics_file, "\nDynamic Energy statistics:\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through every task in the task-set.
    {
        float max = -FLT_MAX, min = FLT_MAX, avg = 0;
        Task task = tasks[i];
        float dynamic_energy;

        fprintf(statistics_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++) // Iterates through each task instance of the chosen task.
        {
            dynamic_energy = task.dynamic_energy_consumed[i];

            fprintf(statistics_file, "%0.5f, ", dynamic_energy);

            avg += dynamic_energy;
            if (dynamic_energy > max)
                max = dynamic_energy;
            if (dynamic_energy < min)
                min = dynamic_energy;
        }
        fprintf(statistics_file, "\n\t(Max: %0.5f, Min:%0.5f, Avg: %0.5f)\n", max, min, avg / task.num_instances);
    }

    return;
}


/*
 * Pre-condition: Valid response time and execution time data from jobs that finished execution.
 * Post-condition: Finds the task-wise response, execution and waiting time and the statistics associated with them.
 */
void 
capture_and_print_task_statistics()
{
    // Allocate memory to hold the data.
    for (int i = 0; i < num_tasks; i++)
    {
        tasks[i].response_times = (float *) calloc(tasks[i].num_instances, sizeof(float));
        tasks[i].execution_times = (float *) calloc(tasks[i].num_instances, sizeof(float));
        tasks[i].execution_freq_indices = (int *) calloc(tasks[i].num_instances, sizeof(int));
        tasks[i].dynamic_energy_consumed = (float *) calloc(tasks[i].num_instances, sizeof(float));
    }

    // Holding a index variable for every task.
    int *task_indices = (int *) calloc(num_tasks, sizeof(int));
    int task_index;
    
    // Capture the response times of the 
    for (int i = 0; i < num_jobs; i++)
    {
        task_index = jobs[i].sorted_task_num;

        tasks[task_index].response_times[task_indices[task_index]] = jobs[i].finish_time - jobs[i].arrival_time;
        tasks[task_index].execution_times[task_indices[task_index]] = jobs[i].aet;
        tasks[task_index].execution_freq_indices[task_indices[task_index]] = jobs[i].execution_freq_index;
        tasks[task_index].dynamic_energy_consumed[task_indices[task_index]] = jobs[i].dynamic_energy_consumed;

        task_indices[task_index]++;
    }
    free(task_indices);

    fprintf(statistics_file, "------------------------------------------------------------\n");
    fprintf(statistics_file, "Printing Task Statistics after Execution.\n");
    fprintf(statistics_file, "Number of tasks: %d\n", num_tasks);

    // Printing the various statistics of execution.
    print_response_times();
    print_response_time_jitters();
    print_execution_times();
    print_waiting_times();
    print_execution_freqs();
    print_dynamic_energy_consumed();

    return;
}


/*
 * Pre-condition: The task-set array containing the phase of each task.
 * Post-condition: The largest phase value of each task in the task-set.
 */
long
find_max_phase()
{
    long max = 0; // Min possible value of phase is 0.
    for (int i = 0; i < num_tasks; i++) // Iterating through each task in the task-set.
    {
        if (max < tasks[i].phase)
            max = tasks[i].phase;
    }

    return max;
}


/*
 * Pre-condition: Variables containing the heap data.
 * Post-condition: Frees and deallocates the heap data.
 */
void
delete_tasks()
{
    // Freeing the heap data inside task data.
    for (int i = 0; i < num_tasks; i++) // Iterating through each task in the task-set.
    {
        free(tasks[i].response_times);
        free(tasks[i].execution_times);
        free(tasks[i].execution_freq_indices);
        free(tasks[i].dynamic_energy_consumed);
    }
    
    // Freeing the task data itself.
    free(tasks);
    
    return;
}