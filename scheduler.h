// Functions related to schedulers in general.
void start_scheduler(); // Initialises the data-structures and variables related to the scheduler.
void scheduler(); // The scheduler itself which adds, runs and completes jobs.
void print_ready_queue(); // To capture the state of the ready queue.
void print_finished_jobs(); // Prints all the jobs that have finished running.

// Functions related to the jobs being executed.
void find_execution_time_periodic_job(); // Finds the execution time of the job using psuedo-random numbers.
void run_job(); // Runs the job till the next decision point.
void insert_job_ready_queue(); // Inserts the job into the ready queue using insertion sort.
void add_job(); // Adds a job to the ready queue once a new job has arrived.
void complete_job(); // Completes the job once it has finished executing.

// Functions related to DVFS algorithms.
void allocate_time(); // Allocates time (available till the next deadline) to jobs based on priority.
void select_frequency(); // Selects the best fit freq and voltage to save as much energy as possible.
void find_next_deadline(); // At any given time, finds the next deadline.
int find_next_decision_point(); // At any given time, finds the next decision point.
void add_dynamic_energy(); // Adds the dynamic power consumed by the current job to the total.