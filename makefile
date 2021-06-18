CC = g++
CFLAGS = -fpie -fopenmp -g --std=c++11 -pthread
LDFLAGS = -latomic
SRC_DIR = src/
TARGET_DIR = bin/

all: Free Lock Benchmark
	rm -f *.o

Benchmark: LockQueue.o DoubleLockQueue.o SCQ_bit.o NCQ.o FIFO_Queue.o FIFO_NCQ_Queue.o benchmark.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

Free: SCQ_bit.o NCQ.o FIFO_Queue.o FIFO_NCQ_Queue.o main_LF.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

Lock: LockQueue.o main_Lock.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

#main_alt: DoubleLockQueue.o LockQueue.o NCQ.o SCQ.o main_alt.o
#	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)
#
#main: LockQueue.o NCQ.o SCQ.o main.o
#	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

%.o: $(SRC_DIR)%.cpp $(SRC_DIR)%.h
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f bin/*
	rm -f *.o
