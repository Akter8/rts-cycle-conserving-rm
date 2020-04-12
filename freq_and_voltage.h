typedef struct
{
    float freq; // Relative to Fmax.
    float voltage; // Absolute value of voltage.
}
Freq_and_voltage;

// Functions.
void input_sort_print_freq_and_voltage(); // Calls all the other functions related to initialising data.
void input_freq_and_voltage(); // Inputs the data related to frequency and voltages.
int sort_freq_and_voltage_comparator(); // The comparator used to compare two instances of Freq_and_voltage.
void sort_freq_and_voltage(); // Used to sort the array containing the frequency and voltage data according to increasing order of frequency.
void print_freq_and_voltage(); // Prints the array of structures containing the frequency and voltage data.
void delete_freq_and_voltage(); // Deallocates and frees the memory allocated to array of structures containing the frequency and voltage data.

void find_static_freq_and_voltage(); // Used to find the static frequency and voltage for the task-set.