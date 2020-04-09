# Variable declaration.
CC = gcc
flags = -c -Wall
executableName = test
driver = driver
output_file = output_file.txt


# Make.
all: $(driver).o task.o
	$(CC)  $(driver).o task.o -o $(executableName)

$(driver).o: $(driver).c
	$(CC) $(flags) $(driver).c

task.o: task.c
	$(CC) $(flags) task.c


# Clean.
clean:
	rm -f *.o $(executableName) $(output_file)