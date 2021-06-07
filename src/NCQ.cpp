/**
 * Scalable Implementation of the Circular Queue (NCQ)
 */

#include "NCQ.h"
#include <iostream>
#include <atomic>
#include <cstddef>

NCQ::NCQ(int capacity) : 
    size(capacity),
    head(new std::atomic<size_t>(capacity)),
    tail(new std::atomic<size_t>(capacity)),
    F_INDEX(2*capacity - 1)
{
    this->threshold = (std::atomic<int>*)malloc(sizeof(std::atomic<int>));
    this->threshold->store(-1);
    Entry_NCQ init_entry = {0, 0};
    for (size_t i = 0; i < 2*capacity; i++){
        entries.push_back(new std::atomic<Entry_NCQ>(init_entry));
    }
    
    // TODO: Are the entries initialized properly?
}

NCQ::~NCQ(){};

// Possibly add copy constructor

void NCQ::print_entry(int j){
    Entry_NCQ ent = entries[j]->load();
    std::cout << "Entry: " << j << std::endl;
    std::cout << "Index: " << ent.index << std::endl;
    std::cout << "Cycle: " << ent.cycle << std::endl;
    std::cout << "----------------------------------------\n";
}

int NCQ::cycle(int x){
    return (x / (this->size));
}

bool NCQ::enq(int index){
    size_t T;
    size_t j;
    Entry_NCQ ent;
    Entry_NCQ new_entry;
    do
    {
        retry:;
        T = tail->load();
        j = T % this->size;
        ent = entries[j]->load();
        if(ent.cycle == cycle(T)){
            //Compare and swap
            tail->compare_exchange_weak(T, T + 1);
            //Goto 6
            goto retry;
        }
        if(ent.cycle + 1 != cycle(T)){
            //Goto 6
            goto retry;
        }
        new_entry = Entry_NCQ(cycle(T), index); 
    } while (!entries[j]->compare_exchange_weak(ent, new_entry));
    return tail->compare_exchange_weak(T, T+1);
}

int NCQ::deq(int * error_code){
    *error_code = 0;
    size_t H;
    size_t j;
    Entry_NCQ ent;
    do
    {
        retry:;
        H = head->load();
        j = H % this->size;
        ent = entries[j]->load();
        if(ent.cycle != cycle(H)){
            if(ent.cycle + 1 == cycle(H)){
                *error_code = -1;
                return -1;
            }
            goto retry;
        }
        
        /* code */
    } while (!head->compare_exchange_weak(H, H + 1));
    return ent.index;
}