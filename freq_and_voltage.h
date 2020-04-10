typedef struct
{
    float freq; // Relative to Fmax.
    float voltage; // Absolute value of voltage.
}
Freq_and_voltage;


void input_freq_and_voltage();
int sort_freq_and_voltage_comparator();
void sort_freq_and_voltage();
void print_freq_and_voltage();
void delete_freq_and_voltage();

void find_static_freq_and_voltage();