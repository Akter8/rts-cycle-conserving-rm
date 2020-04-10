#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "task.h"

extern int num_tasks;
extern Task *tasks;

extern FILE *input_freq_file;
extern int num_freq;
extern float *freq;

extern FILE *output_file;


/*
 * Pre-condition:
 * Post-condition:
 */
void
input_freq()
{
    fscanf(input_freq_file, "%d", &num_freq);

    freq = (float *) malloc(sizeof(float) * num_freq);

    for (int i = 0; i < num_freq; i++)
    {
        fscanf(input_freq_file, "%f", &freq[i]);

        if (freq[i] <= 0 || freq[i] > 1)
        {
            fprintf(stderr, "Invalid input in frequency input file. Please enter valid data\n");
            exit(0);
        }

    }
    

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
int
sort_freq_comparator(const void *a, const void *b)
{
    float freq_a, freq_b;
    freq_a = *(float *) a;
    freq_b = *(float *) b;

    return (freq_a > freq_b) - (freq_a < freq_b);
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
sort_freq()
{
    qsort (freq, num_freq, sizeof(float), sort_freq_comparator);

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
void
print_freq()
{
    fprintf(output_file, "------------------------------------------------------------\n");
    fprintf(output_file, "Frequencies available (relative to Fmax).\n");
    fprintf(output_file, "Number of frequencies: %d\n", num_freq);
    for (int i = 0; i < num_freq; i++)
    {
        fprintf(output_file, "%0.2f, ", freq[i]);
    }
    fprintf(output_file, "\n");

    return;
}


/*
 * Pre-condition:
 * Post-condition:
 */
float
gcd(float a, float b)
{
    if (b == 0) {
        return a;
    }

    return gcd(b, fmod(a,b));
}


/*
 * Pre-condition:
 * Post-condition:
 */
float 
lcm(float a, float b) {
    return (a * b) / gcd(a,b);
}


/*
 * Pre-condition:
 * Post-condition:
 */
float
find_hyperperiod()
{
    int current_lcm = 1;

    for (int i = 0; i < num_tasks; i++)
    {
        current_lcm = lcm(current_lcm, tasks[i].period);
    }

    return current_lcm;
}