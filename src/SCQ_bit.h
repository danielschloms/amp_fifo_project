#ifndef SCQ_BIT_H
#define SCQ_BIT_H

#include <queue>
#include <memory>
#include <atomic>
#include <limits>
#include <vector>
#include <cstddef>
#include "Queue.h"

struct Entry{
    std::atomic<uint64_t> * entr;

    // Padding for different cache line
    // Line size is 64 bytes
    int pad[16];
};

class SCQ : public Queue{
private:
    size_t size;
    uint64_t F_INDEX;
    std::atomic<int> * threshold;
    std::atomic<size_t> * head;
    std::atomic<size_t> * tail;
    std::vector<std::atomic<Entry>*> entries;
    std::vector<Entry> entries_lli;

public:
    SCQ(int capacity);                  // Constructor
    ~SCQ();                             // Destructor
    SCQ(const SCQ & scq);               // Copy Constructor
    bool enq(uint64_t index) override;       // Enqueue operation
    int deq(int * error_code) override; // Dequeue operation
    void catchup(size_t t, size_t h);
    int cycle(int x);
    bool is_big_endian();
    void print_entry(int j);
    bool entry_empty(int j);
    bool is_empty;
};

#endif //SCQ_BIT_H