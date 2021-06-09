#ifndef LockQueue_H
#define LockQueue_H

#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE 1024
#endif

#include <memory>       // For unique pointers
#include <mutex>        // C++ locks
#include "Queue.h"

class LockQueue : public Queue {
private:
    int head;
    int tail;
    int size;
    int num_elements;
    std::unique_ptr<int[]> items;
    //Lock lock;
    std::mutex * lock;
    
public:
    LockQueue(int capacity);                   // Constructor
    LockQueue(const LockQueue & q);           // Copy Constructor
    ~LockQueue();                               // Destructor
    LockQueue& operator=(const LockQueue & q);  // Assignment Operator

    bool enq(int index) override;                            // Enqueue operation
    int deq(int * error_code) override;                  // Dequeue operation

    void kill();

};

#endif //LockQueue_H