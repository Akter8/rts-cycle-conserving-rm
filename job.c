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
 * Pre-condition: The valid task-set containing the number of instances for every task.
 * Post-condition: The total number of task instances that have to execute.
 */
void
calculate_num_jobs()
{
    num_jobs = 0;

    for (int i = 0; i < num_tasks; i++)
    {
        num_jobs += tasks[i].num_instances;
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
    calculate_num_jobs();
    jobs = (Job *) malloc(sizeof(Job) * num_jobs);

    int index = 0;
    for (int i = 0; i < num_tasks; i++) // Iterating through each task of the task-set.
    {
        Task task = tasks[i];
        for (int j = 0; j < task.num_instances; j++) // Iterating through every instance of the selected task.
        {
            jobs[index].task_num = task.task_num;
            jobs[index].sorted_task_num = i;
            jobs[index].instance_num = j;
            jobs[index].arrival_time = task.phase + j * task.period;
            jobs[index].absolute_deadline = jobs[index].arrival_time + task.deadline;
            jobs[index].wcet = task.wcet;

            jobs[index].alive = true;

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

    if (job_a.arrival_time != job_b.arrival_time)
    {
        return job_a.arrival_time - job_b.arrival_time;
    }
    else
    {
        Task task_a, task_b;
        task_a = tasks[job_a.sorted_task_num];
        task_b = tasks[job_b.sorted_task_num];

        if (task_a.period != task_b.period)
        {
            return task_a.period - task_b.period;
        }
        else
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
    for (int i = 0; i < num_jobs; i++)
    {
        Job job = jobs[i];

        fprintf(output_file, "Job-J%d,%d: Arrival time: %0.1f, WCET: %0.1f, Deadline: %0.1f\n", job.task_num, job.instance_num, job.arrival_time, job.wcet, job.absolute_deadline);
    }
    fprintf(output_file, "\n");

    return;
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