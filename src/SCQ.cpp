/**
 * Scalable Implementation of the Circular Queue (SCQ)
 */

#include "SCQ.h"
#include <iostream>
#include <atomic>

SCQ::SCQ(int capacity) : 
    size(capacity),
    head(new std::atomic<int>(2*capacity)),
    tail(new std::atomic<int>(2*capacity)),
    threshold(new std::atomic<int>(-1))
{
    for (size_t i = 0; i < 2*capacity; i++){
        entries.push_back(new std::atomic<Entry>);
    }
    // TODO: Are the entries initialized properly?
}

SCQ::~SCQ(){};

// Possibly add copy constructor

void SCQ::catchup(int t, int h){
    while (!tail->compare_exchange_weak(t, h)){
        h = head->load();
        t = tail->load();
        if (t >= h){
            break;
        }
    }
}

int SCQ::cycle(int x){
    return x / this->size;
}

bool SCQ::enq(int index){
    if (index == F_INDEX){
        std::cout << "Value can't be magic number" << std::endl;
        return false;
    }
    //std::cout << "Try enq\n";
    while (true){
        int t = tail->fetch_add(1); 
        // In the pseudocode, cache_remap is used to reduce false sharing
        // j = cache_remap(T % (2*n))
        // TODO: Check if padding in Entry struct works
        int j = t % (2*this->size);

        // Pseudocode in the paper uses goto, so I do as well
        entry_load_enq:

        Entry ent = entries[j]->load();

        if (ent.cycle < cycle(t) && 
        ent.index == F_INDEX &&
        (ent.is_safe == 1 || head->load() <= t)){
            Entry new_entry(cycle(t), 1, index);
            if (!entries[j]->compare_exchange_weak(ent, new_entry)){
                goto entry_load_enq;
            }
            if (threshold->load() != 3*this->size - 1){
                threshold->store(3*this->size - 1);
            }
            return true;
        }
    }
}

int SCQ::deq(int * error_code){
    // Checks if queue is empty
    *error_code = 1;
    if (threshold->load() < 0){
        return F_INDEX;
    }

    while (true){
        int h = head->fetch_add(1);
        int j = h % (2*this->size);
        entry_load_deq:
        Entry ent = entries[j]->load();
        if (ent.cycle == cycle(h)){
            // TODO: Is this equivalent to ORing with (0, 0, F_INDEX) ?
            auto current = entries[j]->load();
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

        int t = tail->load();
        if (t <= h + 1){
            catchup(t, h+1);
            threshold->fetch_add(-1);
            return F_INDEX;
        }
        if (threshold->fetch_add(-1) <= 0){
            return F_INDEX;
        }
    }
}