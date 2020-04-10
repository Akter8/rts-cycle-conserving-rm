typedef struct
{
    int task_num;
    long phase;
    long period;
    long deadline;
    float wcet; // Worst-case execution time.
    float *response_times;
    float *execution_times;
    int *execution_freq_indices;
    int num_instances;
}
Task;

// Functions.
void create_tasks();
void input_tasks();
void print_tasks();
int sort_tasks_comparator(const void *, const void *);
void sort_tasks();
long find_max_phase();
void delete_tasks();

// Functions related to the statistics of execution.
void capture_and_print_task_statistics();
void print_response_times();
void print_response_time_jitters();
void print_execution_times();
void print_waiting_times();
void print_execution_freqs();
