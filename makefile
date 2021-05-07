CC = g++
CFLAGS = -g -Wall -pthread
LDFLAGS =
SRC_DIR = src/
TARGET_DIR = bin/

all: main

main: FIFO_Lock.o main.o
	$(CC) $(CFLAGS) -o $(TARGET_DIR)$@ $^ $(LDFLAGS)
	rm -f *.o

%.o: $(SRC_DIR)%.cpp $(SRC_DIR)%.h
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm -f bin/*
	rm -f *.o
