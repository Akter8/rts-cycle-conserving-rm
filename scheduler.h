// Related to schedulers in general.
void start_scheduler();
void find_execution_time_periodic_job();
int find_next_decision_point();
void scheduler();
void run_job();
void print_ready_queue();
void allocate_time();
int sort_ready_queue_comparator(const void *, const void *);
void sort_ready_queue();

// Related to the jobs being executed.
void add_job();
void complete_job();
void preempt_job();

// Related to DVFS algorithms.
void select_frequency();
void find_next_deadline();
void add_power();