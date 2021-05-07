#ifndef FIFO_LOCK_H
#define FIFO_LOCK_H

#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE 1024
#endif

#include <memory>       // For unique pointers
#include <mutex>        // C++ locks

class FIFO_Lock{
private:
    int head;
    int tail;
    int size;
    int num_elements;
    std::unique_ptr<int[]> items;
    //Lock lock;
    std::mutex * lock;
    
public:
    FIFO_Lock(int capacity);                    // Constructor
    FIFO_Lock(const FIFO_Lock & q);             // Copy Constructor
    ~FIFO_Lock();                               // Destructor
    FIFO_Lock& operator=(const FIFO_Lock & q);  // Assignment Operator

    bool enq(int x);                            // Enqueue operation
    int deq(int * error_code);                  // Dequeue operation

};

#endif //FIFO_LOCK_H