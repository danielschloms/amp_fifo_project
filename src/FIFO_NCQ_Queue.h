#ifndef FIFO_NCQ_QUEUE_H
#define FIFO_NCQ_QUEUE_H


// Frontend for the NCQ
#include "NCQ.h"

class FIFO_NCQ{
    
private:
    NCQ *aq;
    NCQ *fq;
    int size;
    int *data;

public:
    bool enq(int data);
    int deq(int *error_code);
    FIFO_NCQ(int capacity);
};

#endif //FIFO_QUEUE_H