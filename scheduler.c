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
extern FILE *statistics_file;

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
extern long num_freq_calculations;
extern long num_freq_changes;

Job *ready_queue;
int num_job_in_ready_queue;
int current_job_ready_queue_index;
int current_job_overall_job_index;

extern long end_of_execution_time;
float current_time;
long next_deadline;
float next_decision_point;

long num_context_switches;
long num_preemptions;
long num_cache_impact_points;
int prev_task_instance;
int prev_task;
int current_task_instance;
int current_task;
int prev_return_value;

extern float total_dynamic_energy;


/*
 * Pre-condition: Arrays containing tasks, jobs and freq (along with corresponding voltage values) in a sorted manner. Also requires the static freq and voltage previously found.
 * Post-condition: Starts the dynamic scheduler and initialises the relevant data-structures.
 */
void
start_scheduler()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Scheduler starting. (Scheduling from t=0 to t=%ld).\n\n", end_of_execution_time);

    // Setting the seed before random numbers are generated.
    srand(time(NULL));

    // Initialising variables.
    num_job_in_ready_queue = 0;
    ready_queue = (Job *) malloc(sizeof(Job) * num_job_in_ready_queue);
    current_job_overall_job_index = -1;
    current_job_ready_queue_index = -1;
    current_time = 0;

    num_context_switches = 0;
    num_preemptions = 0;
    num_cache_impact_points = 0;
    prev_task = -1;
    current_task = -1;
    prev_return_value = -1;

    current_freq_and_voltage = freq_and_voltage[static_freq_and_voltage_index];
    current_freq_and_voltage_index = static_freq_and_voltage_index;
    num_freq_changes = 0;
    num_freq_calculations = 0;

    total_dynamic_energy = 0;

    // Starting the scheduler.
    scheduler(); // Since the scheduler is recursive, one call to the scheduler is enough.

    print_finished_jobs();

    // Once the scheduler has finished scheduling.
    free(ready_queue);

    fprintf(output_file, "\n\nScheduler has finished scheduling.\n");
    fprintf(output_file, "\nDisclaimer: Please open the statistics file to view the statistics of the execution of the task set.\n");

    // Printing statistics.
    fprintf(statistics_file, "------------------------------------------------------------\n");
    fprintf(statistics_file, "Total dynamic-energy consumer: %0.2f\n", total_dynamic_energy);
    fprintf(statistics_file, "Weighted average percentage of execution of jobs: %0.1f\n", find_avg_percentage_execution());
    fprintf(statistics_file, "Total number of context-switches: %ld\n", num_context_switches);
    fprintf(statistics_file, "Total number of preemptions: %ld\n", num_preemptions);
    fprintf(statistics_file, "Total number of cache-impact points: %ld\n", num_cache_impact_points);
    fprintf(statistics_file, "Total number of frequency calculations: %ld\n", num_freq_calculations);
    fprintf(statistics_file, "Total number of frequency changes: %ld\n", num_freq_changes);

    // Print job statistics.
    capture_and_print_task_statistics();

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

    // Checking if the periods are equal.
    if (tasks[job_a.sorted_task_num].period != tasks[job_b.sorted_task_num].period)
    {
        return tasks[job_a.sorted_task_num].period - tasks[job_b.sorted_task_num].period;
    }
    else
    {
        // Sorting on absolute deadline.
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
        fprintf(output_file, "Job J%d,%d: Arrival time: %ld, WCET: %0.1f, AET: %0.1f, execution left: %0.1f, time executed: %0.1f, Deadline: %ld\n", job.task_num, job.instance_num, job.arrival_time, job.wcet, job.aet, job.time_left, job.time_executed, job.absolute_deadline);
    }

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
    float time_left = next_deadline - current_time;
    // fprintf(output_file, "next deadline: %ld, current time: %0.2f\n", next_deadline, current_time);

    // Jobs are already sorted based on period/priority in the ready queue.
    // Allocating time to each task based on priority.
    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        if (ready_queue[i].aet - ready_queue[i].time_executed <= time_left) // If more allocation can be done after this allocation.
        {
            ready_queue[i].time_next_execution =  ready_queue[i].aet - ready_queue[i].time_executed;
            time_left = time_left - ready_queue[i].aet - ready_queue[i].time_executed;
            // fprintf(output_file, "%0.2f\n", ready_queue[i].time_next_execution);
        }
        else // If this is the last non-zero allocation.
        {
            // fprintf(output_file, "time left = %0.2f\n", time_left);
            ready_queue[i].time_next_execution = time_left;
            time_left = 0;
            // fprintf(output_file, "%0.2f\n", ready_queue[i].time_next_execution);
        }
    }
    
    return;
}


/*
 * Pre-condition: Sorted frequency and voltage array, also requires time allocated for every task  in the ready queue.
 * Post-condition: The frequency and voltage at which the next job runs. Updates current time with the given overheads as well.
 */
void
select_frequency()
{
    // Finding the time left till the next deadline.
    find_next_deadline();
    float time_left = next_deadline - current_time;
    float dynamic_task_utilisation = 0;

    float prev_freq = current_freq_and_voltage.freq;
    
    // Finding the task utilisation at the current time till the next deadline.
    for (int i = 0; i < num_job_in_ready_queue; i++)
    {
        dynamic_task_utilisation += ready_queue[i].time_next_execution;
    }
    dynamic_task_utilisation = dynamic_task_utilisation / time_left;
    
    // Based on the current task utilisation, we calculate the best fit frequency.

    if (dynamic_task_utilisation >= freq_and_voltage[num_freq_levels - 1].freq) // The case when the task utilisation is >= Fmax.
    {
        current_freq_and_voltage_index = num_freq_levels - 1;
    }
    else // Else if the task utilisation is < 1.
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

    // Adding the freq calculation overhead.
    current_time += FREQUENCY_CALCULATION_OVERHEAD;
    num_freq_calculations++;

    // Not every frequency calculation might lead to a frequency change.

    // If frequency and voltage are changing in next execution as compared to the previous execution.
    if (prev_freq != freq_and_voltage[current_freq_and_voltage_index].freq)
    {
        current_freq_and_voltage = freq_and_voltage[current_freq_and_voltage_index];
        fprintf(output_file, "Frequency change overhead being added. %0.2f + %0.2f = %0.2f\n", current_time, FREQUENCY_CHANGE_OVERHEAD, current_time + FREQUENCY_CHANGE_OVERHEAD);

        // Adding the freq change overhead.
        current_time += FREQUENCY_CHANGE_OVERHEAD;
        num_freq_changes++;

        fprintf(output_file, "Frequency change. New task utilisation at: %0.2f, frequency: %0.2f\n", dynamic_task_utilisation, current_freq_and_voltage.freq);
    }
    else // If the frequency has not changed as compared to the previous execution.
    {
        fprintf(output_file, "No frequency change. New task utilisation at: %0.2f, frequency: %0.2f\n", dynamic_task_utilisation, current_freq_and_voltage.freq);
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

    if (min_deadline > next_deadline && current_time < next_deadline) // If the next_deadline found in the previous call is still the next_deadline. (Might occur when this job has already finished and moved out of the ready queue).
    {
        min_deadline = next_deadline;
    }

    next_deadline = min_deadline;

    return;
}


/*
 * Pre-condition: Job queue sorted on arrival time and index of currently running job in the ready queue.
 * Post-condition: Finds the next decision point between finished execution of current job vs arrival of the next job. Returns a value to specify whether the previous job got to finish executing (return value = 1) or if the previous job was preempted by a new job (return value = 2).
 */
int
find_next_decision_point()
{
    // Checking if this is the last job to execute.
    if ((current_job_overall_job_index != num_jobs - 1) && (current_job_overall_job_index != -1)) // If not last job and not first job.
    {
        if (num_job_in_ready_queue == 0) // If the ready queue is empty.
        {
            next_decision_point = jobs[current_job_overall_job_index + 1].arrival_time;
            return 2;
        }

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
        if (num_job_in_ready_queue == 0) // If the ready queue is empty.
        {
            next_decision_point = end_of_execution_time;
            return 1;
        }
        // fprintf(output_file, "current job index = %d, num ready queue: %d\n", current_job_overall_job_index, num_job_in_ready_queue);

        next_decision_point += ready_queue[current_job_ready_queue_index].time_next_execution;
        return 1;
    }
}


/*
 * Pre-condition: A new job that just entered the ready queue.
 * Post-condition: Finds the actual execution time for the new job and updates its metadata.
 */
void
find_execution_time_periodic_job()
{
    // Actual execution time = (50 to 100)% of the worst-case execution time.
    float aet = rand() % (100 - MIN_PERCENT_EXECUTION);
    aet = (aet + MIN_PERCENT_EXECUTION) / 100;
    aet = aet * ready_queue[num_job_in_ready_queue - 1].wcet;
    ready_queue[num_job_in_ready_queue - 1].aet = aet;
    ready_queue[num_job_in_ready_queue - 1].time_left = aet;

    fprintf(output_file, "Job J%d,%d: Execution time left to finish: %0.2f\n", ready_queue[num_job_in_ready_queue - 1].task_num, ready_queue[num_job_in_ready_queue - 1].instance_num, ready_queue[num_job_in_ready_queue - 1].aet);

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

    // Changing the meta data of the job and the ready queue.
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

    if (current_job_overall_job_index == num_jobs - 1 && num_job_in_ready_queue == 0) // If the job queues are empty.
        return;

    fprintf(output_file, "Job J%d,%d: Finished execution at t=%0.2f. Dynamic energy consumed: %0.2f.\n", ready_queue[current_job_ready_queue_index].task_num, ready_queue[current_job_ready_queue_index].instance_num, current_time, ready_queue[current_job_ready_queue_index].dynamic_energy_consumed);

    // Searching for the same job in the jobs queue so as to update it with run-time metadata.
    for (int i = 0; i < num_jobs; i++)
    {
        if (jobs[i].task_num == ready_queue[current_job_ready_queue_index].task_num && jobs[i].instance_num == ready_queue[current_job_ready_queue_index].instance_num)
        {
            jobs[i].finish_time = current_time;
            jobs[i].time_executed = ready_queue[current_job_ready_queue_index].aet;
            jobs[i].aet = ready_queue[current_job_ready_queue_index].aet;
            jobs[i].alive = false;
            jobs[i].execution_freq_index = current_freq_and_voltage_index;
            jobs[i].dynamic_energy_consumed = ready_queue[current_job_ready_queue_index].dynamic_energy_consumed;

            break;
        }
    }

    // Shifting the jobs in the ready queue by 1 position as the job that finished has to be overwritten.
    for (int i = current_job_ready_queue_index + 1; i < num_job_in_ready_queue; i++)
    {
        ready_queue[i-1] = ready_queue[i];
    }
    
    num_job_in_ready_queue--;
    ready_queue = (Job *) realloc(ready_queue, sizeof(Job) * num_job_in_ready_queue);

    return;
}


/*
 * Pre-condition: The next decision point and the ready queue.
 * Post-condition: Runs and updates the meta data of the highest priority job in the ready queue.
 * 
 * This function does NOT run the IDLE job. (That is taken care of by the scheduler() function).
 */
void
run_job()
{
    if (current_job_overall_job_index == num_jobs - 1 && num_job_in_ready_queue == 0) // If all the jobs are completed.
        return;

    // Finding the next execution time.
    int return_value = find_next_decision_point(); 
    float execution_time = next_decision_point - current_time;

    current_task = ready_queue[current_job_ready_queue_index].task_num;
    current_task_instance = ready_queue[current_job_ready_queue_index].instance_num;

    if ((prev_return_value == 2) && ((current_task != prev_task) || (current_task_instance != prev_task_instance && current_task == prev_task))) // If there was a preemption when the latest job arrived to the ready queue.
    {
        fprintf(output_file, "Job J%d,%d was preempted.\n", prev_task, prev_task_instance);
        num_preemptions++;
    }

    if (prev_task != current_task) // A cache impact point can happen even when there was no preemption, but a voluntary context switch.
            num_cache_impact_points++;

    // Adding the dynamic power consumed by this execution to the total.
    add_dynamic_energy();

    // Updating meta-data.
    ready_queue[current_job_ready_queue_index].time_executed += execution_time;
    ready_queue[current_job_ready_queue_index].time_next_execution -= execution_time;
    ready_queue[current_job_ready_queue_index].time_left -= execution_time;

    fprintf(output_file, "Job J%d,%d: Executed from t=%0.2f to t=%0.2f. Time left after current execution: %0.2f\n", ready_queue[current_job_ready_queue_index].task_num, ready_queue[current_job_ready_queue_index].instance_num, current_time, next_decision_point, ready_queue[current_job_ready_queue_index].aet - ready_queue[current_job_ready_queue_index].time_executed);

    current_time = next_decision_point;

    if (return_value == 1) // If the job was completed.
    {
        ready_queue[current_job_ready_queue_index].time_left = 0;
        // printf("running\n");
        complete_job();
        // printf("terminated.\n");
    }
    else if(return_value == 2) // If the job was interrupted by the arrival of a new job.
    {
        fprintf(output_file, "Job J%d,%d was interrupted by a new job arrival at t=%0.2f.\n", ready_queue[current_job_ready_queue_index].task_num, ready_queue[current_job_ready_queue_index].instance_num, current_time);
    }

    prev_task = current_task;
    prev_task_instance = current_task_instance;
    prev_return_value = return_value;

    fprintf(output_file, "\n");

    return;
}


/*
 * Pre-condition: The array containing all the jobs.
 * Post-condition: Prints the list of jobs (task num and instance number) of the ones that have completed.
 */
void
print_finished_jobs()
{
    fprintf(output_file, "\nPrinting finished jobs.\n");
    int count = 0;
    for (int i = 0; i < num_jobs; i++) // Iterating through all the jobs.
    {
        if (jobs[i].alive == false) // The job is not alive if it has completed.
        {
            fprintf(output_file, "J%d,%d ", jobs[i].task_num, jobs[i].instance_num);
            count++;
        }
    }
    fprintf(output_file, "\nNumber of finished jobs: %d\n", count);

    // If all the jobs are done.
    if (count == num_jobs)
    {
        fprintf(output_file, "All the jobs are done.\n");
    }
    else
    {
        fprintf(output_file, "%d number of jobs are still left.\n", num_jobs);
        fprintf(output_file, "List of jobs still left: ");
        for (int i = 0; i < num_jobs; i++)
        {
            if (jobs[i].alive == true)
                fprintf(output_file, "J%d,%d ", jobs[i].task_num, jobs[i].instance_num);
        }
        
    }
    

    return;
}


/*
 * Pre-condition: A variable containing the previous total dynamic power, and the job that just executed in the ready queue.
 * Post-condition: Updates the total dynamic power by adding the dynamic power consumer by the latest job that executed.
 */
void
add_dynamic_energy()
{
    Job job = ready_queue[current_job_ready_queue_index];

    // Finding the freq and voltage at which the current job executed.
    float execution_freq = freq_and_voltage[current_freq_and_voltage_index].freq;
    float execution_voltage = freq_and_voltage[current_freq_and_voltage_index].voltage;

    // Dynamic power consumed = v * v * f * c. (Let c = 1 is constant).
    // Dynamic energy consumed = Dynamic power * time.
    // Dynamic energy = v * v * f * time.
    float dynamic_energy = execution_voltage * execution_voltage * execution_freq * job.time_next_execution;

    total_dynamic_energy += dynamic_energy;

    ready_queue[current_job_ready_queue_index].dynamic_energy_consumed += dynamic_energy;

    fprintf(output_file, "Dynamic energy consumed by J%d,%d: %0.2f\n", job.task_num, job.instance_num, dynamic_energy);

    return;
}


/*
 * Pre-condition: Ready queue, list of jobs yet to arrive, next deadline and next decision points.
 * Post-condition: Adds, runs and completes jobs in the right order of priority of RM. Also adds to the total dynamic energy consumed.
 */
void
scheduler()
{
    if (current_time >= end_of_execution_time) // If the maximum time of execution has been reached.
    {
        current_time = end_of_execution_time;
        return;
    }

    if (current_job_ready_queue_index == 0 && current_job_overall_job_index == num_jobs - 1) // If all the jobs are completed.
    {
        find_next_decision_point();
        find_next_deadline();
        current_time = next_decision_point;
        return;
    }
    
    int jobs_added = 0; // To find the number of jobs added this call of the scheduler.

    // Adding jobs to the ready queue.
    while (1)
    {
        // If the jobs are done, then we stop searching.
        if (current_job_overall_job_index >= num_jobs - 1)
        {
            break;
        }

        // If a job has arrived.
        if (jobs[current_job_overall_job_index + 1].admitted == false && current_time >= jobs[current_job_overall_job_index + 1].arrival_time) // Since jobs queue is sorted based on arrival time.
        {
            add_job();
            find_execution_time_periodic_job();
            jobs_added++;
        }
        else // If no more jobs are left.
        {
            fprintf(output_file, "\n");
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
        // Finding the min freq and voltage possible to run the idle job.
        current_freq_and_voltage_index = 0;
        current_freq_and_voltage = freq_and_voltage[0];

        find_next_decision_point();

        fprintf(output_file, "Idle job running at lowest frequency and voltage from t=%0.2f to %0.2f.\n", current_time, next_decision_point);

        // fprintf(output_file, "Decision making overhead being added. %0.2f + %0.2f = %0.2f\n", current_time, DECISION_MAKING_OVERHEAD, current_time + DECISION_MAKING_OVERHEAD);
        fflush(output_file);

        current_time += DECISION_MAKING_OVERHEAD;
        current_time = next_decision_point;
        
        if (num_job_in_ready_queue == 0 && current_job_overall_job_index >= num_jobs - 1) // If the final job has completed.
            return;
        else
        {
            num_context_switches++;
            scheduler();
        }
    }

    if (num_job_in_ready_queue == 0 && current_job_overall_job_index >= num_jobs - 1) // If the final job has completed.
        return;

    // Sorting ready queue before allocating time and finding dynamic freq.
    sort_ready_queue();
    // print_ready_queue();

    fprintf(output_file, "Decision making overhead being added. %0.2f + %0.2f = %0.2f\n", current_time, DECISION_MAKING_OVERHEAD, current_time + DECISION_MAKING_OVERHEAD);
    // Adding the decisioin making time.
    current_time += DECISION_MAKING_OVERHEAD;

    allocate_time();
    select_frequency();

    // All the jobs that run from the ready queue will be from index 0 of the ready queue as the ready queue is sorted based on priority (which is the period in case of RM).
    current_job_ready_queue_index = 0;
    run_job();

    if (num_job_in_ready_queue > 0 && current_job_overall_job_index < num_jobs - 1) // If this was not the last job to execute.
        num_context_switches++;

    // print_finished_jobs();
    
    scheduler(); // Recursively calling the scheduler to run the next job.

    return;
}
