#include <stdio.h>
#include <stdlib.h>

#include "configuration.h"
#include "task.h"

extern FILE *input_file;
extern FILE *output_file;

extern int num_tasks;
extern Task *tasks;

/*
 * Pre-condition:
 * Post-condition:
 */
void
create_tasks()
{
    fscanf(input_file, "%d", &num_tasks);

    tasks = (Task *) malloc(sizeof(Task) * num_tasks);

    return;
}

/*
 * Pre-condition:
 * Post-condition:
 */
void
input_tasks()
{
    for (int i = 0; i < num_tasks; i++)
    {
        tasks[i].task_num = i;
        fscanf(input_file, "%f %f %f %f", &tasks[i].phase, &tasks[i].period, &tasks[i].wcet, &tasks[i].deadline);
    }

    return;
}

/*
 * Pre-condition:
 * Post-condition:
 */
void
print_tasks()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Task-set Information (sorted based on period).\n");
    for (int i = 0; i < num_tasks; i++)
    {
        Task task = tasks[i];
        fprintf(output_file, "Task-%d: Phase: %0.1f, Period: %0.1f, WCET: %0.1f, Deadline: %0.1f\n", task.task_num, task.phase, task.period, task.wcet, task.deadline);
    }

    return;
}

/*
 * Pre-condition:
 * Post-condition:
 */
int
sort_tasks_comparator(const void *a, const void *b)
{
    float period_a, period_b;
    period_a = ((Task *) a) -> period;
    period_b = ((Task *) b) -> period;

    return period_a - period_b;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
sort_tasks()
{
    qsort((void *) tasks, num_tasks, sizeof(tasks[0]), sort_tasks_comparator);

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
delete_tasks()
{
    free(tasks);
    
    return;
}