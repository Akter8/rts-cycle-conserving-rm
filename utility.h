typedef struct
{
    float freq; // Relative to Fmax.
    float voltage; // Absolute value of voltage.
}
Freq_and_voltage;

void open_files();
void close_files();
void file_not_null_check();

void input_freq_and_voltage();
int sort_freq_and_voltage_comparator();
void sort_freq_and_voltage();
void print_freq_and_voltage();

float gcd(float, float);
float lcm(float, float);
void find_hyperperiod();

void find_first_in_phase_time();
void find_end_of_execution_time();
void calculate_num_instances_of_tasks();

float floatAbs(float);