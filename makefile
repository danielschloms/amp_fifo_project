CC = g++
CFLAGS = -g -fPIE -fopenmp --std=c++11 -Wall -pthread
LDFLAGS = -latomic
SRC_DIR = src/
TARGET_DIR = bin/

all: main

main: LockQueue.o SCQ.o main.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)
	rm -f *.o

%.o: $(SRC_DIR)%.cpp $(SRC_DIR)%.h
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f bin/*
	rm -f *.o
