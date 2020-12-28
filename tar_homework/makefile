CC=gcc
FLAGS=-o
all: run


compile:
	$(CC) main.c  $(FLAGS) check


run: compile
	./check tar -c main.c tar_check1.cpp main.bin
	./check tar -h main.c main.bin

	echo files in tar:
	./check tar -l main.bin

	./check tar -u tar_check2.c main.bin
	echo after update:
	./check tar -l main.bin

	./check tar -d main.c main.bin
	echo after delete:
	./check tar -l main.bin