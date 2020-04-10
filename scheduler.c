#include <stdio.h>
#include <stdlib.h>

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


/*
 * Pre-condition: Arrays containing tasks, jobs and freq (along with corresponding voltage values) in a sorted manner. Also requires the static freq and voltage previously found.
 * Post-condition: Starts the dynamic scheduler.
 */
void
start_scheduler()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Scheduler starting.\n");


    return;
}