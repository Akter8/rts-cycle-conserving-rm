#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include "configuration.h"
#include "scheduler.h"
#include "task.h"
#include "job.h"
#include "utility.h"
#include "freq_and_voltage.h"

extern FILE *output_file;

extern int num_tasks;
extern Task *tasks;

extern int num_jobs;
extern Job *jobs;

extern int num_freq_levels;
extern Freq_and_voltage *freq_and_voltage;

extern int static_freq_and_voltage_index;
extern Freq_and_voltage static_freq_and_voltage;
Freq_and_voltage current_freq_and_voltage;
int current_freq_and_voltage_index;
extern int num_freq_calculations;
extern int num_freq_changes;

Job *ready_queue;
int num_job_in_ready_queue;
int current_job_ready_queue_index;
int current_job_overall_job_index;

float current_time;
int next_deadline;

int num_preemptions;
int num_cache_impact_points;


/*
 * Pre-condition: Arrays containing tasks, jobs and freq (along with corresponding voltage values) in a sorted manner. Also requires the static freq and voltage previously found.
 * Post-condition: Starts the dynamic scheduler and initialises the relevant data-structures.
 */
void
start_scheduler()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Scheduler starting.\n");

    srand(time(NULL));

    num_job_in_ready_queue = 0;
    ready_queue = (Job *) malloc(sizeof(Job) * num_job_in_ready_queue);
    current_job_overall_job_index = -1;
    current_job_ready_queue_index = -1;
    current_time = 0;

    num_preemptions = 0;
    num_cache_impact_points = 0;

    current_freq_and_voltage = freq_and_voltage[static_freq_and_voltage_index];
    current_freq_and_voltage_index = static_freq_and_voltage_index;
    num_freq_changes = 0;
    num_freq_calculations = 0;

    // current_time = 2;
    // find_next_deadline();
    // current_time = 4;
    // find_next_deadline();
    // current_time = 6;
    // find_next_deadline();
    // current_time = 15;
    // find_next_deadline();

    return;
}


/*
 * Pre-condition: Two job instances from the ready queue.
 * Post-condition: The comparision value of comparing the two given jobs.
 * 
 * Sorts based on period of corresponding task. If periods are same, then sorts based on absolute deadline of the jobs. 
 */
int
sort_ready_queue_comparator(const void *a, const void *b)
{
    Job job_a, job_b;
    job_a = *((Job *) a);
    job_b = *((Job *) b);

    if (tasks[job_a.sorted_task_num].period != tasks[job_b.sorted_task_num].period)
    {
        return tasks[job_a.sorted_task_num].period - tasks[job_b.sorted_task_num].period;
    }
    else
    {
        return job_a.absolute_deadline - job_b.absolute_deadline;
    }
}


/*
 * Pre-condition: Unsorted ready queue.
 * Post-condition: Sorted ready queue.
 */
void
sort_ready_queue()
{
    qsort(ready_queue, num_job_in_ready_queue, sizeof(Job), sort_ready_queue_comparator);

    return;
}


/*
 * Pre-condition: Ready queue containing jobs.
 * Post-condition: Allocates amount of time that each job from the ready will run till the next decision-point.
 */
void
allocate_time()
{
    find_next_deadline();
    int time_left = next_deadline - current_time;

    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        if (ready_queue[i].aet - ready_queue[i].time_executed < time_left)
        {
            ready_queue[i].time_next_execution =  ready_queue[i].aet - ready_queue[i].time_executed;
            time_left = time_left - ready_queue[i].aet - ready_queue[i].time_executed;
        }
        else
        {
            ready_queue[i].time_next_execution = time_left;
            time_left = 0;
        }
    }
    
    return;
}


/*
 * Pre-condition: Sorted frequency and voltage array.
 * Post-condition: The frequency and voltage at which the next job runs. Updates current time with the given overheads as well.
 */
void
select_frequency()
{
    allocate_time();
    float time_left = next_deadline - current_time;

    float dynamic_task_utilisation = 0;
    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        dynamic_task_utilisation += ready_queue[i].time_next_execution;
    }
    dynamic_task_utilisation = dynamic_task_utilisation / time_left;
    

    for (int i = 0; i < num_freq_levels; i++)
    {
        if (dynamic_task_utilisation >= freq_and_voltage[i].freq)
        {
            current_freq_and_voltage_index = i;
        }
        else
        {
            break;
        }
        
    }

    current_time += FREQUENCY_CALCULATION_OVERHEAD;
    num_freq_calculations++;

    // If frequency and voltage are changing in next execution.
    if (current_freq_and_voltage.freq == freq_and_voltage[current_freq_and_voltage_index].freq)
    {
        current_freq_and_voltage = freq_and_voltage[current_freq_and_voltage_index];
        current_time += FREQUENCY_CHANGE_OVERHEAD;
        num_freq_changes++;
    }

    return;
}


/*
 * Pre-condition: Sorted ready queue.
 * Post-condition: Prints the ready queue onto the output file.
 */
void
print_ready_queue()
{
    fprintf(output_file, "\n\nPrinting ready queue.\n");
    fprintf(output_file, "Number of jobs in the ready queue: %d\n", num_job_in_ready_queue);
    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        Job job = ready_queue[i];
        fprintf(output_file, "Job J%d,%d: Arrival time: %ld, WCET: %0.1f, AET: %0.1f Deadline: %ld\n", job.task_num, job.instance_num, job.arrival_time, job.wcet, job.aet, job.absolute_deadline);
    }

    return;
}


/*
 * Pre-condition: Ready queue.
 * Post-condition: Finds the closest deadline greater than current_time.
 */
void
find_next_deadline()
{
    long min_deadline = LONG_MAX;
    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        if (current_time < ready_queue[i].absolute_deadline && ready_queue[i].absolute_deadline < min_deadline)
        {
            min_deadline = ready_queue[i].absolute_deadline;
        }
    }
    // printf("Current time: %0.1f, next deadline: %ld\n", current_time, min_deadline);

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
find_next_decision_point()
{


    return;
}


/*
 * Pre-condition: A new job that just entered the ready queue.
 * Post-condition: Finding the actual execution time for the new job.
 */
void
find_execution_time_periodic_job()
{
    Job job = ready_queue[current_job_ready_queue_index];
    
    float aet = rand() % (100 - MIN_PERCENT_EXECUTION);
    aet = (aet + MIN_PERCENT_EXECUTION) / 100;
    aet = aet * job.wcet;
    job.aet = aet;

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
run_job()
{


    return;
}


/*
 * Pre-condition: Job which needs to be added to the ready queue.
 * Post-condition: Adds the new job onto ready queue and calculates the new frequency.
 */
void
add_job()
{
    Job job = jobs[current_job_overall_job_index];
    current_job_overall_job_index++;

    job.admitted = true;
    num_job_in_ready_queue++;
    ready_queue = (Job *) realloc(ready_queue, sizeof(Job) * num_job_in_ready_queue);
    ready_queue[num_job_in_ready_queue - 1] = job;
    
    select_frequency();
    
    return;
}


/*
 * Pre-condition: Job that needs to be removed from the ready queue.
 * Post-condition: Removes the job from ready queue, updates job's metadata, finds new frequency at which next job runs.
 */
void
complete_job()
{
    Job job = ready_queue[current_job_ready_queue_index];

    job.alive = false;
    num_job_in_ready_queue--;
    ready_queue = (Job *) realloc(ready_queue, sizeof(Job) * num_job_in_ready_queue);

    for (int i = 0; i < num_jobs; i++)
    {
        if (jobs[i].task_num == job.task_num && jobs[i].instance_num == job.instance_num)
        {
            jobs[i].finish_time = current_time;
            jobs[i].time_executed = job.aet;
            jobs[i].alive = false;
            jobs[i].execution_freq = current_freq_and_voltage.freq;

            break;
        }
    }
    
    select_frequency();

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
preempt_job()
{


    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
scheduler()
{


    return;
}