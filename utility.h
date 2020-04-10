// Functions to help with files.
void open_files();
void close_files();
void file_not_null_check();

// General Utility functions.
float gcd(float, float);
float lcm(float, float);
float floatAbs(float);

// Functions related to finding meta-data of the task-set before execution starts.
void find_hyperperiod();
void find_first_in_phase_time();
void find_end_of_execution_time();
void calculate_num_instances_of_tasks();
float find_task_utilisation();