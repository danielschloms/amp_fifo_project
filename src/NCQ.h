#ifndef NCQ_H
#define NCQ_H

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

struct Entry_NCQ{
    int cycle;
    int index;

    // Needed for atomic
    Entry_NCQ() = default;

    Entry_NCQ(int c, int idx){
        cycle = c;
        index = idx;
    }

    // Struct must be trivially copyable for atomic
    Entry_NCQ(Entry_NCQ const& e) = default;

    // Padding for different cache line
    // Line size is 64 bytes
    int pad[14];
};

class NCQ : public Queue{
private:
    int size;
    int F_INDEX;
    std::atomic<int> * threshold;
    std::atomic<size_t> * head;
    std::atomic<size_t> * tail;
    std::vector<std::atomic<Entry_NCQ>*> entries;

public:
    NCQ(int capacity);                  // Constructor
    ~NCQ();                             // Destructor
    NCQ(const NCQ & ncq);               // Copy Constructor
    bool enq(int index) override;       // Enqueue operation
    int deq(int * error_code) override; // Dequeue operation
    int cycle(int x);
    void print_entry(int j);
    bool entry_empty(int j);
    bool is_empty;
};

#endif //NCQ_H