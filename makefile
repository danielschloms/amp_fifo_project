CC = g++
CFLAGS = -fpie -Wall -fopenmp -g --std=c++11 -pthread
LDFLAGS = -latomic
SRC_DIR = src/
TARGET_DIR = bin/

all: Demo LockDemo
	rm -f *.o

Benchmark: LockQueue.o DoubleLockQueue.o SCQ_bit.o NCQ.o FIFO_SCQ_Queue.o FIFO_NCQ_Queue.o benchmark.o
	mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

Demo: SCQ_bit.o NCQ.o FIFO_SCQ_Queue.o FIFO_NCQ_Queue.o main_LF.o
	mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

LockDemo: LockQueue.o main_Lock.o
	mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

%.o: $(SRC_DIR)%.cpp $(SRC_DIR)%.h
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f bin/*
	rm -f *.o