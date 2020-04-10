#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "configuration.h"
#include "utility.h"
#include "task.h"
#include "job.h"

extern FILE *input_tasks_file;
extern FILE *output_file;

extern int num_tasks;
extern Task *tasks;

extern float end_of_execution_time;

extern int num_jobs;
extern Job *jobs;

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
        fscanf(input_tasks_file, "%f %f %f %f", &tasks[i].phase, &tasks[i].period, &tasks[i].wcet, &tasks[i].deadline);

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
        fprintf(output_file, "Task-%d: Phase: %0.1f, Period: %0.1f, WCET: %0.1f, Deadline: %0.1f, Number of instances: %d\n", task.task_num, task.phase, task.period, task.wcet, task.deadline, task.num_instances);
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

    if (task_a.period != task_b.period)
    {
        return task_a.period - task_b.period;
    }
    else
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
    fprintf(output_file, "\nResponse time statistics:\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through each task.
    {
        float max = -FLT_MAX, min = FLT_MAX, avg = 0;
        Task task = tasks[i];
        float response_time;

        fprintf(output_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++) // Iterates through each instance of the task.
        {
            response_time = task.response_times[j];

            fprintf(output_file, "%0.1f, ", response_time);

            avg += response_time;
            if (response_time > max)
                max = response_time;
            if (response_time < min)
                min = response_time;
        }
        fprintf(output_file, "\n\t(Max: %0.1f, Min:%0.1f, Avg: %0.1f)\n", max, min, avg / task.num_instances);
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
    fprintf(output_file, "\nResponse time jitter statistics.\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through each task in the task-set.
    {
        Task task = tasks[i];

        fprintf(output_file, "Task-%d: Absolute RTJ: ", task.task_num);
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
        fprintf(output_file, "%0.1f, Relative RTJ: ", max - min);

        // If the number of instances in the hyperperiod is only 1, then the relative RTJ will be 0.
        if (task.num_instances == 1)
        {
            fprintf(output_file, "0.0\n");
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


        fprintf(output_file, "%0.1f\n", max);
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
    fprintf(output_file, "\nExecution time statistics:\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through each task in the task set.
    {
        float max = -FLT_MAX, min = FLT_MAX, avg = 0;
        Task task = tasks[i];
        float execution_time;

        fprintf(output_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++) // Iterates through every instance of the given task.
        {
            execution_time = task.execution_times[j];

            fprintf(output_file, "%0.1f, ", execution_time);

            avg += execution_time;
            if (execution_time > max)
                max = execution_time;
            if (execution_time < min)
                min = execution_time;
        }
        fprintf(output_file, "\n\t(Max: %0.1f, Min:%0.1f, Avg: %0.1f)\n", max, min, avg / task.num_instances);
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
    fprintf(output_file, "\nExecution time statistics:\n");
    for (int i = 0; i < num_tasks; i++)
    {
        float max = -FLT_MAX, min = FLT_MAX, avg = 0;
        Task task = tasks[i];
        float waiting_time;

        fprintf(output_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++)
        {
            waiting_time = task.response_times[j] - task.execution_times[j];

            fprintf(output_file, "%0.1f, ", waiting_time);

            avg += waiting_time;
            if (waiting_time > max)
                max = waiting_time;
            if (waiting_time < min)
                min = waiting_time;
        }
        fprintf(output_file, "\n\t(Max: %0.1f, Min:%0.1f, Avg: %0.1f)\n", max, min, avg / task.num_instances);
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
    fprintf(output_file, "\nExecution frequency statistics:\n");
    for (int i = 0; i < num_tasks; i++) // Iterates through each task in the task set.
    {
        float max = -FLT_MAX, min = FLT_MAX, avg = 0;
        Task task = tasks[i];
        float execution_freq;

        fprintf(output_file, "Task-%d: ", task.task_num);
        for (int j = 0; j < task.num_instances; j++) // Iterates through each task instance of the given task.
        {
            execution_freq = task.execution_freqs[j];

            fprintf(output_file, "%0.1f, ", execution_freq);

            avg += execution_freq;
            if (execution_freq > max)
                max = execution_freq;
            if (execution_freq < min)
                min = execution_freq;
        }
        fprintf(output_file, "\n\t(Max: %0.1f, Min:%0.1f, Avg: %0.1f)\n", max, min, avg / task.num_instances);
    }

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void 
capture_and_print_task_statistics()
{
    // Allocate memory to hold the data.
    for (int i = 0; i < num_tasks; i++)
    {
        tasks[i].response_times = (float *) calloc(tasks[i].num_instances, sizeof(float));
        tasks[i].execution_times = (float *) calloc(tasks[i].num_instances, sizeof(float));
        tasks[i].execution_freqs = (float *) calloc(tasks[i].num_instances, sizeof(float));
    }

    // Holding a index variable for every task.
    int *task_indices = (int *) calloc(num_tasks, sizeof(int));
    int task_index;
    
    // Capture the response times of the 
    for (int i = 0; i < num_jobs; i++)
    {
        task_index = jobs[i].sorted_task_num;

        tasks[task_index].response_times[task_indices[task_index]] = jobs[i].finish_time;
        tasks[task_index].execution_times[task_indices[task_index]] = jobs[i].aet;

        task_indices[task_index]++;
    }
    free(task_indices);

    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Printing Task Statistics after Execution.\n");
    fprintf(output_file, "Number of tasks: %d\n", num_tasks);

    // Printing the various statistics of execution.
    print_response_times();
    print_response_time_jitters();
    print_execution_times();
    print_waiting_times();
    print_execution_freqs();

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
delete_tasks()
{
    // Freeing the heap data inside task data.
    for (int i = 0; i < num_tasks; i++)
    {
        free(tasks[i].response_times);
        free(tasks[i].execution_times);
        free(tasks[i].execution_freqs);
    }
    
    // Freeing the task data itself.
    free(tasks);
    
    return;
}