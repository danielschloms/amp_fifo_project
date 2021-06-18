#ifndef FIFO_SCQ_QUEUE_H
#define FIFO_SCQ_QUEUE_H

// Frontend for the SCQ
#include "SCQ_bit.h"

class FIFO_SCQ{
    
private:
    SCQ *aq;
    SCQ *fq;
    int size;
    int *data;
    bool after = false;
    bool run = true;
    

public:
    bool enq(int data);
    int deq(int *error_code);
    FIFO_SCQ(int capacity);
    std::atomic<size_t> num_thread[32];
    void refresh();
    void terminate();
    void check_entries(bool caq);
};

#endif //FIFO_SCQ_QUEUE_H