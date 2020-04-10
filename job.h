#include <stdbool.h>

typedef struct
{
    int task_num;
    int sorted_task_num;
    int instance_num;
    float arrival_time;
    float wcet; // Worst-case execution time.
    float aet; // Actual execution time.
    float absolute_deadline;
    float finish_time;
    float execution_freq;
    bool alive;
}
Job;


// Functions.
void calculate_num_jobs();
void create_jobs();
int sort_jobs_comparator(const void *, const void *);
void sort_jobs();
void print_jobs();
void delete_jobs();