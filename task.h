typedef struct
{
    // To identify the task.
    int task_num;

    // Timing characteristics of the task.
    long phase; // First arrival of the task instance of this task.
    long period;
    long deadline;
    float wcet; // Worst-case execution time.

    // To find the statistics later on for the task using all task instances.
    float *response_times;
    float *execution_times;
    int *execution_freq_indices;
    int num_instances;
}
Task;

// Functions.
void create_tasks(); // Allocates memory for task array.
void input_tasks(); // Takes input and initialises the task array.
void print_tasks(); // Prints the task-set info.
int sort_tasks_comparator(const void *, const void *); // Comparator used to sort the task-set.
void sort_tasks(); // Sorts task array based on period (and priority in the case of RM).
long find_max_phase(); // Finds the largest phase of the task-set (to help with finding the end time of execution).
void delete_tasks(); // Deallocates and frees the data used by the task array when no longer needed.

// Functions related to the statistics of execution.
void capture_and_print_task_statistics(); // Calls all the other print statistics functions for the task set.
void print_response_times(); // Finds and prints the response times of all the jobs in the task set.
void print_response_time_jitters(); // Finds and prints the response time jitters.
void print_execution_times(); // Finds and prints the execution times of all the jobs in the task-set.
void print_waiting_times(); // Finds and prints the waiting times of all the jobs in the task-set.
void print_execution_freqs(); // Finds and prints the execution frequencies and voltages of all jobs in the task set.
