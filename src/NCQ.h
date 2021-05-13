#ifndef NCQ_H
#define NCQ_H

#include <queue>
#include <memory>

class NCQ{
private:
    std::queue<int> aq;             // Indices of allocated entries
    std::queue<int> fq;             // Indices of unallocated entries
    std::unique_ptr<int*[]> items;  // Array of pointers
    int size;
    signed int threshold;
    int head;
    int tail;

public:
    NCQ(int capacity);      // Constructor
    bool enq(int index);    // Enqueue operation
    int deq();              // Dequeue operation
};

#endif //NCQ_H