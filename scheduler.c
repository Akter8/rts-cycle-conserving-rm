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

extern long end_of_execution_time;
float current_time;
long next_deadline;
float next_decision_point;

int num_preemptions;
int num_cache_impact_points;

extern float total_power;


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

    total_power = 0;

    current_time = 0.25;
    find_next_deadline();
    // current_time = 4;
    // find_next_deadline();
    // current_time = 6;
    // find_next_deadline();
    // current_time = 15;
    // find_next_deadline();

    current_time = 0;
    scheduler();

    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Total dynamic-power consumer: %0.2f\n", total_power);
    fprintf(output_file, "Total number of preemptions: %d\n", num_preemptions);
    fprintf(output_file, "Total number of cache-impact points: %d\n", num_cache_impact_points);
    fprintf(output_file, "Total number of frequency calculations: %d\n", num_freq_calculations);
    fprintf(output_file, "Total number of frequency changes: %d\n", num_freq_changes);


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

    // Jobs are already sorted based on period/priority in the ready queue.
    // Allocating time to each task based on priority.
    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        if (ready_queue[i].aet - ready_queue[i].time_executed < time_left) // If more allocation can be done after this allocation.
        {
            ready_queue[i].time_next_execution =  ready_queue[i].aet - ready_queue[i].time_executed;
            time_left = time_left - ready_queue[i].aet - ready_queue[i].time_executed;
            printf("%0.2f\n", ready_queue[i].time_next_execution);
        }
        else // If this is the last non-zero allocation.
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
    find_next_deadline();
    float time_left = next_deadline - current_time;
    float dynamic_task_utilisation = 0;

    float prev_freq = current_freq_and_voltage.freq;

    if (num_job_in_ready_queue == 0)
    {
        fprintf(output_file, "zero jobs\n");      
    }
    
    // Finding the task utilisation at the current time.
    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        dynamic_task_utilisation += ready_queue[i].time_next_execution;
        fprintf(output_file, "%0.2f\n", ready_queue[i].time_next_execution);
    }
    dynamic_task_utilisation = dynamic_task_utilisation / time_left;
    

    if (dynamic_task_utilisation >= freq_and_voltage[num_freq_levels - 1].freq)
    {
        current_freq_and_voltage_index = num_freq_levels - 1;
    }
    else
    {
        // Finding the highest frequency that satisfies the task-utilisation.
        for (int i = num_freq_levels - 1; i >= 0; i--)
        {
            if (dynamic_task_utilisation <= freq_and_voltage[i].freq)
            {
                current_freq_and_voltage_index = i;
            }
            else
            {
                break;
            }
        }
    }

    fprintf(output_file, "Frequency calculation overhead being added. %0.2f + %0.2f = %0.2f\n", current_time, FREQUENCY_CALCULATION_OVERHEAD, current_time + FREQUENCY_CALCULATION_OVERHEAD);
    current_time += FREQUENCY_CALCULATION_OVERHEAD;
    num_freq_calculations++;

    // If frequency and voltage are changing in next execution.
    if (prev_freq != freq_and_voltage[current_freq_and_voltage_index].freq)
    {
        current_freq_and_voltage = freq_and_voltage[current_freq_and_voltage_index];
        fprintf(output_file, "Frequency change overhead being added. %0.2f + %0.2f = %0.2f\n", current_time, FREQUENCY_CHANGE_OVERHEAD, current_time + FREQUENCY_CHANGE_OVERHEAD);
        current_time += FREQUENCY_CHANGE_OVERHEAD;
        num_freq_changes++;
        fprintf(output_file, "Frequency change. New task utilisation at: %0.2f, frequency: %0.2f\n", dynamic_task_utilisation, current_freq_and_voltage.freq);
        fprintf(output_file, "Next deadline: %ld\n", next_deadline);
    }
    else // If the frequency has not changed.
    {
        fprintf(output_file, "No frequency change. New task utilisation at: %0.2f, frequency: %0.2f\n", dynamic_task_utilisation, current_freq_and_voltage.freq);
        fprintf(output_file, "Next deadline: %ld\n", next_deadline);
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
    for (int i = 0; i < num_job_in_ready_queue; i++) // Iterating through every job in the ready queue.
    {
        Job job = ready_queue[i];
        fprintf(output_file, "Job J%d,%d: Arrival time: %ld, WCET: %0.1f, AET: %0.1f, execution left: %0.1f, Deadline: %ld\n", job.task_num, job.instance_num, job.arrival_time, job.wcet, job.aet, job.time_left, job.absolute_deadline);
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

    // Iterate through every job in the ready queue to find the next deadline.
    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        if (current_time < ready_queue[i].absolute_deadline && ready_queue[i].absolute_deadline < min_deadline)
        {
            min_deadline = ready_queue[i].absolute_deadline;
        }
    }
    if (min_deadline > next_deadline && current_time < next_deadline)
        min_deadline = next_deadline;
    printf("Current time: %0.1f, next deadline: %ld\n", current_time, min_deadline);
    next_deadline = min_deadline;

    return;
}


/*
 * Pre-condition: Job queue sorted on arrival time and index of currently running job in the ready queue.
 * Post-condition: Finds the next decision point between finished execution of current job vs arrival of the next job.
 */
int
find_next_decision_point()
{
    // Checking if this is the last job to execute.
    if ((current_job_overall_job_index != num_jobs - 1) && (current_job_overall_job_index != -1)) // If not last job and not first job.
    {
        next_decision_point = (current_time + ready_queue[current_job_ready_queue_index].time_next_execution < jobs[current_job_overall_job_index + 1].arrival_time) ? current_time + ready_queue[current_job_ready_queue_index].time_next_execution : jobs[current_job_overall_job_index + 1].arrival_time;

        if (current_time + ready_queue[current_job_ready_queue_index].time_next_execution < jobs[current_job_overall_job_index + 1].arrival_time)
            return 1;
        else
            return 2;
    }
    else if (current_job_overall_job_index == -1) // If its the first job.
    {
        next_decision_point = jobs[current_job_overall_job_index + 1].arrival_time;
        return 2;
    }
    else // If last job.
    {
        next_decision_point = current_time += ready_queue[current_job_ready_queue_index].time_next_execution;
        return 1;
    }
}


/*
 * Pre-condition: A new job that just entered the ready queue.
 * Post-condition: Finding the actual execution time for the new job.
 */
void
find_execution_time_periodic_job()
{
   
    float aet = rand() % (100 - MIN_PERCENT_EXECUTION);
    aet = (aet + MIN_PERCENT_EXECUTION) / 100;
    aet = aet * ready_queue[num_job_in_ready_queue - 1].wcet;
    ready_queue[num_job_in_ready_queue - 1].aet = aet;
    ready_queue[num_job_in_ready_queue - 1].time_left = aet;

    fprintf(output_file, "Job J%d,%d: Execution time to finish: %0.2f\n", ready_queue[num_job_in_ready_queue - 1].task_num, ready_queue[num_job_in_ready_queue - 1].instance_num, ready_queue[num_job_in_ready_queue - 1].aet);

    return;
}


/*
 * Pre-condition: Job which needs to be added to the ready queue.
 * Post-condition: Adds the new job onto ready queue.
 */
void
add_job()
{
    Job job = jobs[current_job_overall_job_index + 1];
    current_job_overall_job_index++;

    fprintf(output_file, "Job J%d,%d: Added to the ready queue at t=%0.2f.\n", job.task_num, job.instance_num, current_time);

    job.admitted = true;
    num_job_in_ready_queue++;
    ready_queue = (Job *) realloc(ready_queue, sizeof(Job) * num_job_in_ready_queue);
    ready_queue[num_job_in_ready_queue - 1] = job;

    return;
}


/*
 * Pre-condition: Job that needs to be removed from the ready queue.
 * Post-condition: Removes the job from ready queue, updates job's metadata.
 */
void
complete_job()
{
    //---------------------------------------------------------------------------------
    // Update required while removing job from the ready queue.

    fprintf(output_file, "Job J%d,%d: Finished execution at t=%0.2f.\n", ready_queue[current_job_ready_queue_index].task_num, ready_queue[current_job_ready_queue_index].instance_num, current_time);

    // Searching for the same job in the jobs queue so as to update it with run-time metadata.
    for (int i = 0; i < num_jobs; i++)
    {
        if (jobs[i].task_num == ready_queue[current_job_ready_queue_index].task_num && jobs[i].instance_num == ready_queue[current_job_ready_queue_index].instance_num)
        {
            jobs[i].finish_time = current_time;
            jobs[i].time_executed = ready_queue[current_job_ready_queue_index].aet;
            jobs[i].alive = false;
            jobs[i].execution_freq_index = current_freq_and_voltage_index;

            break;
        }
    }

    for (int i = current_job_ready_queue_index + 1; i < num_job_in_ready_queue; i++)
    {
        ready_queue[i-1] = ready_queue[i];
    }
    

    ready_queue[current_job_ready_queue_index].alive = false;
    num_job_in_ready_queue--;
    ready_queue = (Job *) realloc(ready_queue, sizeof(Job) * num_job_in_ready_queue);

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
run_job()
{
    int return_value = find_next_decision_point(); 
    float execution_time = next_decision_point - current_time;
    
    ready_queue[current_job_ready_queue_index].time_executed += execution_time;
    ready_queue[current_job_ready_queue_index].time_next_execution -= execution_time;

    fprintf(output_file, "Job J%d,%d: Executed from t=%0.2f to t=%0.2f. Time left after current execution: %0.5f\n", ready_queue[current_job_ready_queue_index].task_num, ready_queue[current_job_ready_queue_index].instance_num, current_time, next_decision_point, ready_queue[current_job_ready_queue_index].aet - ready_queue[current_job_ready_queue_index].time_executed);

    current_time = next_decision_point;

    if (return_value == 1)
    {
        ready_queue[current_job_ready_queue_index].time_left = 0;
        complete_job();
    }
    

    // fprintf(output_file, "time left: %0.9f\n",  ready_queue[current_job_ready_queue_index].aet - ready_queue[current_job_ready_queue_index].time_executed);
    // if (ready_queue[current_job_ready_queue_index].aet - ready_queue[current_job_ready_queue_index].time_executed <= 0.001)
    // {
    //     fprintf(output_file, "zero\n");
    //     
    // }
    

    return;
}


/*
 * Pre-condition: A variable containing the previous total dynamic power, and the job that just executed in the ready queue.
 * Post-condition: Updates the total dynamic power by adding the dynamic power consumer by the latest job that executed.
 */
void
add_power()
{
    Job job = ready_queue[current_job_ready_queue_index];

    float execution_freq = freq_and_voltage[job.execution_freq_index].freq;
    float execution_voltage = freq_and_voltage[job.execution_freq_index].voltage;

    // Power += v * v * f * time.
    total_power += execution_voltage * execution_voltage * execution_freq * job.time_next_execution;

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
scheduler()
{
    // printf("abcd\n");
    fflush(stdout);
    fflush(output_file);

    // Removing a job from the ready queue.
    // At a time, only one job can finish, hence we do the completion check only once.
    // if (ready_queue[current_job_ready_queue_index].time_left == 0)
    // {
    //     complete_job();
    // }
    
    int jobs_added = 0; // To find the number of jobs added this call of the scheduler.

    // Adding jobs to the ready queue.
    while (1)
    {
        // If the jobs are done, then we stop searching.
        if (current_job_overall_job_index == num_jobs - 1)
        {
            break;
        }

        // If the job has arrived.
        if (jobs[current_job_overall_job_index + 1].admitted == false && current_time >= jobs[current_job_overall_job_index + 1].arrival_time) // Since jobs queue is sorted based on arrival time.
        {
            add_job();
            find_execution_time_periodic_job();
            jobs_added++;
        }
        else // If no more jobs are left.
        {
            break;
        }
    }

    // If the end of execution time (which is equal to min(3*hyperperiod, first inphase time + hyperperiod)) is reached, then we stop scheduling.
    // If the ready queue is empty and all jobs are done, then the scheduler can stop executing.
    if ((current_time >= end_of_execution_time) || (num_job_in_ready_queue == 0 && current_job_overall_job_index == num_jobs - 1))
    {
        fprintf(output_file, "\n\nScheduler has finished.\n");
        return;
    }

    // Checking if the ready queue is empty. Have to run idle job if it is.
    if (num_job_in_ready_queue == 0)
    {
        current_freq_and_voltage_index = 0;
        current_freq_and_voltage = freq_and_voltage[0];

        fprintf(output_file, "Idle job running at lowest frequency and voltage from t=%0.2f to %0.2f.\n", current_time, next_decision_point);

        add_power();

        fprintf(output_file, "Decision making overhead being added. %0.2f + %0.2f = %0.2f\n", current_time, DECISION_MAKING_OVERHEAD, current_time + DECISION_MAKING_OVERHEAD);
        current_time += DECISION_MAKING_OVERHEAD;
        current_time = next_decision_point;
        find_next_decision_point();
        scheduler();
    }
    // else if (jobs_added != 0) // Allocate time should only be called when at least one job has been added to the job queue.
    // {
    //     sort_ready_queue();
    //     allocate_time();
    //     fprintf(output_file, "New time allocated.\n");
    // }
    // printf("Jobs added: %d\n", jobs_added);

    print_ready_queue();
    

    sort_ready_queue();
    allocate_time();
    select_frequency();

    fprintf(output_file, "Decision making overhead being added. %0.2f + %0.2f = %0.2f\n", current_time, DECISION_MAKING_OVERHEAD, current_time + DECISION_MAKING_OVERHEAD);
    current_time += DECISION_MAKING_OVERHEAD;

    current_job_ready_queue_index = 0;
    run_job();

    scheduler();

    return;
}