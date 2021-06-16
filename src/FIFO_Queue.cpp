#include "FIFO_Queue.h"
#include <iostream>

FIFO::FIFO(int capacity){
    size = capacity;
    data = (int*)malloc(capacity*sizeof(int));
    aq = new SCQ(capacity, false);
    fq = new SCQ(capacity, true);
}

int FIFO::deq(int *error_code){
    int ec;
    int val;
    int zero = 0;
    int index = aq->deq(&ec);
    if (index == ~zero) {
        *error_code = -1;
        return 0;
    }
    val = data[index];
    fq->enq(index);
    return val;
}

bool FIFO::enq(int x){
    int ec;
    int index = fq->deq(&ec);
    //std::cout << index << std::endl;
    int zero = 0;

    if (index == ~zero) {
        return false;
    }

    data[index] = x;
    aq->enq(index);
    return true;
}