#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "job.h"
#include "task.h"

extern FILE *output_file;

extern int num_tasks;
extern Task *tasks;

extern int num_jobs;
extern Job *jobs;


/*
 * Pre-condition: Uninitialised variables to hold the data corresponding to the jobs to be scheduled.
 * Post-condition: Creates, initialises, sorts and prints the jobs data.
 * 
 * This function acts as an init_jobs() function and calls all the other initialising functions.
 */
void
create_sort_print_jobs()
{
    // Create task instances.
    create_jobs();

    // Sort jobs.
    sort_jobs();

    // Print the job information.
    print_jobs();

    return;
}


/*
 * Pre-condition: The valid task-set containing the number of instances for every task.
 * Post-condition: The total number of task instances that have to execute.
 */
void
calculate_num_jobs()
{
    num_jobs = 0;

    for (int i = 0; i < num_tasks; i++) // Iterating through each task in the task-set.
    {
        num_jobs += tasks[i].num_instances; // Adding each task's number of instances.
    }

    return;
}


/*
 * Pre-condition: The valid task-set data.
 * Post-condition: An array of initialised data containing job details.
 */
void
create_jobs()
{
    // Allocating space in the heap for the jobs.
    calculate_num_jobs();
    jobs = (Job *) malloc(sizeof(Job) * num_jobs);

    int index = 0;
    // Initialising the task set.
    for (int i = 0; i < num_tasks; i++) // Iterating through each task of the task-set.
    {
        Task task = tasks[i];
        for (int j = 0; j < task.num_instances; j++) // Iterating through every instance of the selected task.
        {
            jobs[index].task_num = task.task_num;
            jobs[index].sorted_task_num = i;
            jobs[index].instance_num = j;
            jobs[index].arrival_time = task.phase + (j * task.period);
            jobs[index].absolute_deadline = jobs[index].arrival_time + task.deadline;
            jobs[index].wcet = task.wcet;

            jobs[index].alive = true;
            jobs[index].admitted = false;
            jobs[index].time_executed = 0;

            jobs[index].execution_freq_index = -1;
            jobs[index].dynamic_energy_consumed = 0;

            jobs[index].aet = -1;
            jobs[index].finish_time = -1;

            index++;
        }
    }
    
    return;
}


/*
 * Pre-condition: Two job instances from the array.
 * Post-condition: The comparator value to help with sorting.
 * 
 * Compares based on arrival time, if the arrival time is same, then compares based period. If the period is also same, then compare based on wcet. 
 */
int 
sort_jobs_comparator(const void *a, const void *b)
{
    Job job_a, job_b;
    job_a = *((Job *) a);
    job_b = *((Job *) b);

    if (job_a.arrival_time != job_b.arrival_time) // If arrival time is not equal, then we compare.
    {
        return job_a.arrival_time - job_b.arrival_time;
    }
    else // Compare based on the period of its corresponding task.
    {
        Task task_a, task_b;
        task_a = tasks[job_a.sorted_task_num];
        task_b = tasks[job_b.sorted_task_num];

        if (task_a.period != task_b.period)
        {
            return task_a.period - task_b.period;
        }
        else // Else compare based on wcet.
        {
            return task_a.wcet - task_b.wcet;
        }
    }
}


/*
 * Pre-condition: An unsorted array of jobs.
 * Post-condition: A sorted array of jobs. Sorted based on the comparator function.
 */
void
sort_jobs()
{
    qsort((void *) jobs, num_jobs, sizeof(jobs[0]), sort_jobs_comparator);

    return;
}


/*
 * Pre-condition: An array containing valid job data.
 * Post-condition: Nothing.
 * 
 * Prints a human readable version of the job data onto the output file.
 */
void
print_jobs()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Job Information.\n");
    fprintf(output_file, "Number of jobs: %d\n", num_jobs);
    for (int i = 0; i < num_jobs; i++) // Iterating through every job.
    {
        Job job = jobs[i];

        fprintf(output_file, "Job-J%d,%d: Arrival time: %ld, WCET: %0.1f, Deadline: %ld\n", job.task_num, job.instance_num, job.arrival_time, job.wcet, job.absolute_deadline);
    }
    fprintf(output_file, "\n");

    return;
}


/*
 * Pre-condition: The job array containint the actual execution time and the wcet.
 * Post-condition: The weighted average of the percent of the percent of execution for the task-set.
 */
float
find_avg_percentage_execution()
{
    float numerator = 0;
    float denominator = 0;

    /*
     *
     *                      sum(number * weight)
     * Weighted average = ------------------------
     *                           sum(weight)
     * 
     * Here, number = percent of actual execution as compared to worst case.
     * and,  weight = wcet.
     * 
     *                      sum(percent of actual execution compared to worst case * worst case execution time)
     * Weighted average = --------------------------------------------------------------------------------------
     *                                                  sum(worst case execution time)
     * 
     *                      sum(actual execution time / worst case execution time * worst case execution time)
     *                  = --------------------------------------------------------------------------------------
     *                                                  sum(worst case execution time)
     * 
     *                       sum(actual execution time)
     *                  = ---------------------------------
     *                      sum(worst case execution time)
     *
     */

    for (int i = 0; i < num_jobs; i++)
    {
        Job job = jobs[i];

        // AET of jobs are initialised as -1. And they get updated only if the job finished.
        if (job.aet < 0) // Same as if(job did not finish).
            continue; // So as to not count them in the total.

        numerator += job.aet;
        denominator += job.wcet;
    }

    return numerator / denominator * 100;
}


/*
 * Pre-condition: An array containing job data.
 * Post-condition: Frees the job data from the heap.
 */
void
delete_jobs()
{
    free(jobs);

    return;
}