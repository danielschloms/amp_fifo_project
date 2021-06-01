#ifndef SCQ_H
#define SCQ_H

#include <queue>
#include <memory>
#include <atomic>
#include <limits>
#include <vector>
#include <cstddef>
#include "Queue.h"

//#ifndef F_INDEX
//#define F_INDEX 2048
//#endif

struct Entry{
    int cycle;
    int is_safe;
    int index;

    // Needed for atomic
    Entry() = default;

    Entry(int c, int is, int idx){
        cycle = c;
        is_safe = is;
        index = idx;
    }

    // Struct must be trivially copyable for atomic
    Entry(Entry const& e) = default;

    // Padding for different cache line
    // Line size is 64 bytes
    int pad[14];
};

class SCQ : public Queue{
private:
    int size;
    int F_INDEX;
    std::atomic<int> * threshold;
    std::atomic<size_t> * head;
    std::atomic<size_t> * tail;
    std::vector<std::atomic<Entry>*> entries;

public:
    SCQ(int capacity);                  // Constructor
    ~SCQ();                             // Destructor
    SCQ(const SCQ & scq);               // Copy Constructor
    bool enq(int index) override;       // Enqueue operation
    int deq(int * error_code) override; // Dequeue operation
    void catchup(size_t t, size_t h);
    int cycle(int x);
    bool is_big_endian();
    void print_entry(int j);
    bool entry_empty(int j);
    bool is_empty;
};

#endif //SCQ_H