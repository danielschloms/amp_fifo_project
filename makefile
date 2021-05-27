CC = g++
CFLAGS = -fpie -fopenmp -g --std=c++11 -Wall -pthread
LDFLAGS = -latomic
SRC_DIR = src/
TARGET_DIR = bin/

all: main main_alt
	rm -f *.o

main_alt: LockQueue.o SCQ.o main_alt.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

main: LockQueue.o SCQ.o main.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)

%.o: $(SRC_DIR)%.cpp $(SRC_DIR)%.h
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f bin/*
	rm -f *.o
