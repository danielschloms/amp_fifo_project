/**
 * Scalable Implementation of the Circular Queue (SCQ)
 */

#include "SCQ_bit.h"
#include <iostream>
#include <atomic>
#include <cstddef>

SCQ::SCQ(int capacity, bool full){
    this->threshold = (std::atomic<int>*)malloc(sizeof(std::atomic<int>));
    this->head = (std::atomic<size_t>*)malloc(sizeof(std::atomic<size_t>));
    this->tail = (std::atomic<size_t>*)malloc(sizeof(std::atomic<size_t>));
    F_INDEX = 2*capacity - 1;
    size = capacity;
    if (full){
        this->threshold->store(3*capacity-1);
        this->head->store(0);
        this->tail->store(2*capacity);
    }
    else{
        this->threshold->store(-1);
        this->head->store(2*capacity);
        this->tail->store(2*capacity);
    }
    for (size_t i = 0; i < 2*capacity; i++){
        Entry e;
        e.entr = new std::atomic<uint64_t>(i);
        if (!full){
            e.entr->fetch_or(2*size - 1);
        }
        else{
            e.entr->fetch_or(2*size >> 1);
        }
        entries_lli.push_back(e);
    }
}

SCQ::~SCQ(){};

// Possibly add copy constructor

void SCQ::catchup(size_t t, size_t h){
    while (!tail->compare_exchange_weak(t, h)){
        h = head->load();
        t = tail->load();
        if (t >= h){
            break;
        }
    }
}

void SCQ::kill(){
    run = false;
}

int SCQ::cycle(int x){
    return x / (size);
}

bool SCQ::entry_empty(int j){
    return entries_lli[j].entr->load() & F_INDEX == F_INDEX;
}

bool SCQ::enq(uint64_t index){
    index ^= (size - 1);
    while (true){
        size_t t = tail->fetch_add(1); 
        // In the pseudocode, cache_remap is used to reduce false sharing
        // j = cache_remap(T % (2*n))
        size_t j = t % (2*size);

        entry_load_enq:
        // Pseudocode in the paper uses goto, so I do as well
        uint64_t ent = entries_lli[j].entr->load();
        
        uint64_t ent_cycle = ent | (2*size - 1);
        uint64_t t_cycle = (t << 1) | (2*size - 1);

        if (ent_cycle < t_cycle && 
            (ent == ent_cycle && (ent == ent_cycle || (ent == ent_cycle ^ size) && head->load() <= t))){
            uint64_t new_entry = t_cycle ^ index; 
            if (!entries_lli[j].entr->compare_exchange_weak(ent, new_entry)){
                goto entry_load_enq;
            }
            
            if (threshold->load() != (3*size) - 1){
                threshold->store((3*size) - 1);
            }
            
            return true;
        }
    }
}

int SCQ::deq(){
    // Checks if queue is empty
    F_INDEX = 2*size-1;
    if (threshold->load() < 0){
        is_empty = true;
        return -1;
    }

    while (true){
        size_t h = head->fetch_add(1);
        size_t j = h % (2*size);

        entry_load_deq:
        uint64_t ent = entries_lli[j].entr->load();
        uint64_t ent_cycle = ent | (2*size - 1);

        uint64_t h_cycle = (h << 1) | (2*size - 1);

        if (ent_cycle == h_cycle){
            uint64_t current = entries_lli[j].entr->load();
            entries_lli[j].entr->fetch_or(size - 1);
            return ent & (size - 1);
        }

        uint64_t new_ent = ent & (~size);

        if (ent | F_INDEX == ent){
            new_ent = h_cycle | (ent & size);
        }

        if (ent_cycle < h_cycle){
            if (!entries_lli[j].entr->compare_exchange_weak(ent, new_ent)){
                goto entry_load_deq;
            }
        }
        
        size_t t = tail->load();
        if (t <= h + 1){
            catchup(t, h+1);
            threshold->fetch_sub(1);
            return -1;
        }

        if (threshold->fetch_sub(1) <= 0){
            is_empty = true;
            return -1;
        }
    }
}