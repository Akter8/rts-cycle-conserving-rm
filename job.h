#include <stdbool.h>

typedef struct
{
    // To identify the job.
    int task_num; // Task number corresponding to the task-num of the task.
    int sorted_task_num; // Index of the task in the task-array which is sorted based on period (because of sorting, sorted_task_num need to be equal to task_num).
    int instance_num;

    // Timing characteristics of the job.
    long arrival_time;
    float wcet; // Worst-case execution time.
    float aet; // Actual execution time.
    float time_executed; // Total amount of time the job has executed for so far.
    float time_next_execution; // Time alloted till next deadline.
    float time_left; // Time left in overall execution. Will be 0 when job has finished executing.
    long absolute_deadline;
    float finish_time;

    // Energy characterstics.
    int execution_freq_index; // Freq and voltage of last execution.
    float dynamic_energy_consumed;

    // To find whether a job has completed or has already been accepted.
    bool alive;
    bool admitted;
}
Job;


// Functions.
void create_sort_print_jobs(); // Calls all the other jobs.
void create_jobs(); // Creates jobs using the task-set info
int sort_jobs_comparator(const void *, const void *); // Comparator used to sort the jobs.
void sort_jobs(); // Jobs are sorted based on arrival time.
void print_jobs(); // To print the information related to the jobs.
void delete_jobs(); // To free the heap data once the execution is complete.

void calculate_num_jobs();
float find_avg_percentage_execution();