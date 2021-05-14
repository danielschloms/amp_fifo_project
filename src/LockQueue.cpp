#include "LockQueue.h"
#include <iostream>

/**
 * Since the std::mutex is non-copyable, the copy-constructor, destructor and the assignment operator
 * have to be implemented explicitly for the class to function properly. This section is not relevant to
 * the actual queue implementation and just handles object functionality.
 */

// Constructor
LockQueue::LockQueue(int capacity) : Queue(),
    head(0), 
    tail(0), 
    size(capacity), 
    lock(new std::mutex) 
    {
        items = std::unique_ptr<int[]>(new int[capacity]);
    }   

// Destructor
LockQueue::~LockQueue() { delete lock; }   

// Copy Constructor
LockQueue::LockQueue( const LockQueue & q ) : Queue(), head(q.head), tail(q.tail), size(q.size), lock(new std::mutex) {
    // Copy item array
    items = std::unique_ptr<int[]>(new int[q.size]);
    for (int i = 0; i < q.size; i++){
        items.get()[i] = q.items.get()[i];
    }
} 

// Assignment Operator
LockQueue& LockQueue::operator=( const LockQueue & q ) {                            
    head=q.head; 
    tail=q.tail;
    size=q.size;
    items = std::unique_ptr<int[]>(new int[q.size]);
    for (int i = 0; i < q.size; i++){
        items.get()[i] = q.items.get()[i];
    }
}                                                                                                         

/**
 *  The relevant (enqueue/dequeue) methods are below 
 */

/**
 * Enqueues an integer.
 * @param x The integer to enqueue
 * @returns True if operation was successful, false if queue was full
 */ 
bool LockQueue::enq(int index){
    lock->lock();
    if (tail - head == this->size){
        // Queue full, unlock and return false
        lock->unlock();
        return false;
    }
    items[tail % this->size] = index;
    this->tail ++;
    // Enqueue successful, unlock and return true
    lock->unlock();
    return true;
}

/**
 * Dequeues an integer from the queue. If the operation was successful, the error
 * code will be set to 0, otherwise -1.
 * @param error_code: Pointer to a location where the error code will be set
 * @returns The dequeued value
 */
int LockQueue::deq(int * error_code){
   lock->lock();
   if (tail == head){
        // Queue empty: unlock, set error code to -1 and return 0
        lock->unlock();
        *error_code = -1;
        return 0;
   }
   int index = items[head % this->size];
   head ++;
   // Dequeue successful: unlock, set error code to 0 and return the value
   lock->unlock();
   *error_code = 0;
   return index;
}