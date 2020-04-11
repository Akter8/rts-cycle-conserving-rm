#include <stdbool.h>

typedef struct
{
    int task_num;
    int sorted_task_num;
    int instance_num;
    long arrival_time;
    float wcet; // Worst-case execution time.
    float aet; // Actual execution time.
    float time_executed;
    float time_next_execution;
    float time_left;
    long absolute_deadline;
    float finish_time;
    int execution_freq_index;
    bool alive;
    bool admitted;
}
Job;


// Functions.
void calculate_num_jobs();
void create_jobs();
int sort_jobs_comparator(const void *, const void *);
void sort_jobs();
void print_jobs();
void delete_jobs();