/**
 * Scalable Implementation of the Circular Queue (SCQ)
 */

#include "SCQ.h"
#include <iostream>
#include <atomic>
#include <cstddef>

SCQ::SCQ(int capacity, bool full){
    this->threshold = (std::atomic<int>*)malloc(sizeof(std::atomic<int>));
    this->head = (std::atomic<size_t>*)malloc(sizeof(std::atomic<size_t>));
    this->tail = (std::atomic<size_t>*)malloc(sizeof(std::atomic<size_t>));
    F_INDEX = 2*capacity - 1;
    //std::cout << "FINDEX: " << F_INDEX << std::endl;
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
    Entry init_entry = {0, 1, F_INDEX};
    for (size_t i = 0; i < 2*capacity; i++){
        entries.push_back(new std::atomic<Entry>(init_entry));
    }
    
    // TODO: Are the entries initialized properly?
}

SCQ::~SCQ(){};

// Possibly add copy constructor

void SCQ::print_entry(int j){
    Entry ent = entries[j]->load();
    std::cout << "Entry " << j << std::endl;
    std::cout << "Index: " << ent.index << std::endl;
    std::cout << "Is Safe: " << ent.is_safe << std::endl;
    std::cout << "Cycle: " << ent.cycle << std::endl;
    std::cout << "----------------------------------------\n";
}

void SCQ::catchup(size_t t, size_t h){
    while (!tail->compare_exchange_weak(t, h)){
        h = head->load();
        t = tail->load();
        if (t >= h){
            break;
        }
    }
}

bool SCQ::entry_empty(int j){
    return entries[j]->load().index == F_INDEX;
}

int SCQ::cycle(int x){
    return (x / (this->size));
}

void SCQ::kill(){
    run = false;
}

bool SCQ::enq(int index){
    // Strange memory bug?
    F_INDEX = 2*size-1;

    while (true){
        size_t t = tail->fetch_add(1); 
        // In the pseudocode, cache_remap is used to reduce false sharing
        // j = cache_remap(T % (2*n))
        // TODO: Check if padding in Entry struct works
        size_t j = t % (2*this->size);

        // Pseudocode in the paper uses goto, so I do as well
        entry_load_enq:

        Entry ent = entries[j]->load();

        if (ent.cycle < cycle(t) && ent.index == F_INDEX && (ent.is_safe == 1 || head->load() <= t)){
            Entry new_entry(cycle(t), 1, index);
            if (!entries[j]->compare_exchange_weak(ent, new_entry)){
                goto entry_load_enq;
            }
            
            if (this->threshold->load() != (3*this->size) - 1){
                this->threshold->store((3*this->size) - 1);
            }
            return true;
        }
    }
}

int SCQ::deq(){
    // Checks if queue is empty
    if (this->threshold->load() < 0){
        is_empty = true;
        return ~0;
    }

    while (true){
        size_t h = head->fetch_add(1);
        size_t j = h % (2*this->size);
        entry_load_deq:
        Entry ent = entries[j]->load();
        if (ent.cycle == cycle(h)){
            Entry current = entries[j]->load();
            Entry or_ent = {current.cycle, current.is_safe, F_INDEX};
            while (!entries[j]->compare_exchange_weak(current, or_ent));
            return ent.index;
        }

        Entry new_entry = {ent.cycle, 0, ent.index};
        if (ent.index == F_INDEX){
            new_entry = {cycle(h), ent.is_safe, F_INDEX};
        }
        if (ent.cycle < cycle(h)){
            if (!entries[j]->compare_exchange_weak(ent, new_entry)){
                goto entry_load_deq;
            }
        }
        
        size_t t = tail->load();
        if (t <= h + 1){
            catchup(t, h+1);
            this->threshold->fetch_sub(1);
            return ~0;
        }

        if (this->threshold->fetch_sub(1) <= 0){
            is_empty = true;
            return ~0;
        }
    }
}