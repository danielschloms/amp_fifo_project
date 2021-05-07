#include "FIFO_Lock.h"
#include <iostream>

/**
 * Since the std::mutex is non-copyable, the copy-constructor, destructor and the assignment operator
 * have to be implemented explicitly for the class to function properly. This section is not relevant to
 * the actual queue implementation and just handles object functionality.
 */

// Constructor
FIFO_Lock::FIFO_Lock(int capacity) : 
    head(0), 
    tail(0), 
    size(capacity), 
    lock(new std::mutex) 
    {
        items = std::unique_ptr<int[]>(new int[capacity]);
    }   

// Destructor
FIFO_Lock::~FIFO_Lock() { delete lock; }   

// Copy Constructor
FIFO_Lock::FIFO_Lock( const FIFO_Lock & q ) : head(q.head), tail(q.tail), size(q.size), lock(new std::mutex) {
    // Copy item array
    items = std::unique_ptr<int[]>(new int[q.size]);
    for (int i = 0; i < q.size; i++){
        items.get()[i] = q.items.get()[i];
    }
} 

// Assignment Operator
FIFO_Lock& FIFO_Lock::operator=( const FIFO_Lock & q ) {                            
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
bool FIFO_Lock::enq(int x){
    lock->lock();
    if (tail - head == this->size){
        // Queue full, unlock and return false
        lock->unlock();
        return false;
    }
    items[tail % this->size] = x;
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
int FIFO_Lock::deq(int * error_code){
   lock->lock();
   if (tail == head){
        // Queue empty: unlock, set error code to -1 and return 0
        lock->unlock();
        *error_code = -1;
        return 0;
   }
   int x = items[head % this->size];
   head ++;
   // Dequeue successful: unlock, set error code to 0 and return the value
   lock->unlock();
   *error_code = 0;
   return x;
}