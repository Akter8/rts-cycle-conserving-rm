// Functions to help with files.
void open_files_and_init_data(); // Master function related to opening files and creating, inputting, sorting and printing the data.
void close_files_and_delete_data(); // Master function related to close files and related to deallocating heap memory.
void files_not_null_check();

// General Utility functions.
float gcd(float, float); // To find the gcd of two numbers.
float lcm(float, float); // To find the lcm of two numbers.
float floatAbs(float); // To find the absolute value of a floating point number.

// Functions related to finding meta-data of the task-set before execution starts.
void find_hyperperiod(); // Calculates the hyperperiod.
void find_first_in_phase_time(); // Calculates the first in-phase time.
void find_end_of_execution_time(); // Calculates the end of execution time.

void calculate_num_instances_of_tasks(); // Calculates the total number of jobs to schedule.
float find_task_utilisation(); // Calculates the worst-case task utilisation of the task-set.