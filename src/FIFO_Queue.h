#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

// Frontend for the SCQ
#include "SCQ.h"

class FIFO{
    
private:
    SCQ *aq;
    SCQ *fq;
    int size;
    int *data;

public:
    bool enq(int data);
    int deq(int *error_code);
    FIFO(int capacity);
};

#endif //FIFO_QUEUE_H