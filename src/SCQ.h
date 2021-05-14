#ifndef SCQ_H
#define SCQ_H

#include <queue>
#include <memory>
#include <atomic>
#include <limits>
#include <vector>
#include "Queue.h"

#ifndef F_INDEX
#define F_INDEX INT32_MIN
#endif

struct Entry{
    int cycle = 0;
    int is_safe = 1;
    int index = F_INDEX;

    // Needed for atomic
    Entry() = default;

    Entry(int c, int is, int idx){
        cycle = c;
        is_safe = is;
        index = idx;
    }

    // Struct must be trivially copyable for atomic
    Entry(Entry const&) = default;

    // Padding for different cache line
    // Line size is 64 bytes
    int pad[14];
};

class SCQ : public Queue{
private:
    int size;
    std::atomic<signed int> * threshold;
    std::atomic<int> * head;
    std::atomic<int> * tail;
    std::vector<std::atomic<Entry>*> entries;

public:
    SCQ(int capacity); // Constructor
    ~SCQ();                 // Destructor
    SCQ(const SCQ & scq);   // Copy Constructor
    bool enq(int index) override;    // Enqueue operation
    int deq(int * error_code) override;              // Dequeue operation
    void catchup(int t, int h);
    int cycle(int x);
    bool is_big_endian();
};

#endif //SCQ_H