#ifndef DoubleLockQueue_H
#define DoubleLockQueue_H

#ifndef DEFAULT_SIZE
#define DEFAULT_SIZE 1024
#endif

#include <memory>       // For unique pointers
#include <mutex>        // C++ locks
#include "Queue.h"

class DoubleLockQueue : public Queue {
private:
    int head;
    int tail;
    int size;
    int num_elements;
    std::unique_ptr<int[]> items;
    //Lock lock;
    std::mutex * enqLock;
    std::mutex * deqLock;
    
public:
    DoubleLockQueue(int capacity);                   // Constructor
    DoubleLockQueue(const DoubleLockQueue & q);           // Copy Constructor
    ~DoubleLockQueue();                               // Destructor
    DoubleLockQueue& operator=(const DoubleLockQueue & q);  // Assignment Operator

    bool enq(int index) override;                            // Enqueue operation
    int deq(int * error_code) override;                  // Dequeue operation

    void kill();

};

#endif //LockQueue_H