# Variable declaration.
CC = gcc
flags = -c -Wall
executableName = test
driver = driver
output_file = output_file.txt


# Make.
all: $(driver).o task.o utility.o
	$(CC)  $(driver).o task.o utility.c -o $(executableName) -lm

$(driver).o: $(driver).c
	$(CC) $(flags) $(driver).c

task.o: task.c
	$(CC) $(flags) task.c

utility.o: utility.c
	$(CC) $(flags) utility.c


# Clean.
clean:
	rm -f *.o $(executableName) $(output_file)