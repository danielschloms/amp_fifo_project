#include "FIFO_NCQ_Queue.h"
#include <iostream>

FIFO_NCQ::FIFO_NCQ(int capacity){
    size = capacity;
    data = (int*)malloc(capacity*sizeof(int));
    aq = new NCQ(capacity, false);
    fq = new NCQ(capacity, true);
}

int FIFO_NCQ::deq(int *error_code){
    int index = aq->deq(error_code);
    std::cout << index << std::endl;
    if (index == -1) {
        *error_code = -1;
        return 0;
    }
    int val = data[index];
    fq->enq(index);
    return val;
}

bool FIFO_NCQ::enq(int x){
    int errorCode = 0;
    int index = fq->deq(&errorCode);
    std::cout << index << std::endl;
    if (index == -1) {
        return false;
    }
    data[index] = x;
    aq->enq(index);
    return true;
}