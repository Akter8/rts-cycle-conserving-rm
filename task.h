typedef struct
{
    int task_num;
    float phase;
    float period;
    float deadline;
    float wcet;
}
Task;

// Functions.
void create_tasks();
void input_tasks();
void print_tasks();
int sort_tasks_comparator(const void *, const void *);
void sort_tasks();
void delete_tasks();
