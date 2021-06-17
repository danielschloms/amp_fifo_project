#include "FIFO_Queue.h"
#include <iostream>

FIFO::FIFO(int capacity){
    size = capacity;
    data = (int*)malloc(capacity*sizeof(int));
    aq = new SCQ(capacity, false);
    fq = new SCQ(capacity, true);
}

int FIFO::deq(int *error_code){
    int index = aq->deq();
    if (index == -1) {
        *error_code = -1;
        return 0;
    }
    int val = data[index];
    fq->enq(index);
    return val;
}

bool FIFO::enq(int x){
    int index = fq->deq();
    //std::cout << index << std::endl;
    if (index == -1) {
        return false;
    }
    data[index] = x;
    aq->enq(index);
    return true;
}