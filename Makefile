# Variable declaration.
CC = gcc
flags = -c -Wall
executableName = test
driver = driver
output_file = output_file.txt
statistics_file = output_statistics_file.txt


# Make.
all: $(driver).o task.o job.o freq_and_voltage.o scheduler.o utility.o
	$(CC)  $(driver).o task.o job.o freq_and_voltage.o scheduler.o utility.o -o $(executableName) -lm

$(driver).o: $(driver).c
	$(CC) $(flags) $(driver).c

task.o: task.c
	$(CC) $(flags) task.c

job.o: job.c
	$(CC) $(flags) job.c

freq_and_voltage.o: freq_and_voltage.c
	$(CC) $(flags) freq_and_voltage.c

scheduler.o: scheduler.c
	$(CC) $(flags) scheduler.c

utility.o: utility.c
	$(CC) $(flags) utility.c


# Clean.
clean:
	rm -f *.o $(executableName) $(output_file) $(statistics_file)