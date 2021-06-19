#include "FIFO_SCQ_Queue.h"
#include <iostream>

FIFO_SCQ::FIFO_SCQ(int capacity){
    size = capacity;
    data = (int*)malloc(capacity*sizeof(int));
    aq = new SCQ(capacity, false);
    fq = new SCQ(capacity, true);
    
    for (int i = 0; i<32; i++){
        num_thread[i].store(0);
    }
}

void FIFO_SCQ::check_entries(bool caq){
    if (caq){
        aq->check_entries();
    }
    else{
        fq->check_entries();
    }
}

void FIFO_SCQ::refresh(){
    aq->refresh();
    fq->refresh();
    after = true;
}

void FIFO_SCQ::terminate(){
    run = false;
    fq->terminate();
    aq->terminate();
}

int FIFO_SCQ::deq(int *error_code){
    int index = aq->deq(error_code);
    if (index == -1) {
        *error_code = -1;
        return 0;
    }
    int val = data[index];
    fq->enq(index);
    return val;
}

bool FIFO_SCQ::enq(int x){
    int error_code;
    int index = fq->deq(&error_code);
    if (index == -1) {
        return false;
    }
    data[index] = x;
    aq->enq(index);
    return true;
}