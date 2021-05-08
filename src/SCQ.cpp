/**
 * Scalable Implementation of the Circular Queue (SCQ)
 */

#include "SCQ.h"
#include <iostream>

/**
 * Determines endianness
 */
bool SCQ::is_big_endian(){
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1; 
}

SCQ::SCQ(int capacity) : 
    size(capacity),
    head(new std::atomic<int>(2*capacity)),
    tail(new std::atomic<int>(2*capacity)),
    threshold(new std::atomic<int>(-1))
{
    items = std::unique_ptr<int*[]>(new int*[capacity]);
    entries = std::unique_ptr<std::atomic<Entry>[]>(new std::atomic<Entry>[2*capacity]);
    // TODO: Are the entries initialized properly?
    big_endian = SCQ::is_big_endian();
}

void SCQ::catchup(int t, int h){
    while (!tail->compare_exchange_strong(t, h)){
        h = head->load();
        t = tail->load();
        if (t >= h){
            break;
        }
    }
}

int SCQ::cycle(int x){
    return x % this->size;
}

void SCQ::enq(int index){
    if (index == F_INDEX){
        std::cout << "Value can't be magic number" << std::endl;
    }
    while (true){
        int t = tail->fetch_add(1);
        // In the pseudocode, cache_remap is used to reduce false sharing
        // j = cache_remap(T % (2*n))
        // TODO: Check if padding in Entry struct works
        int j = t % (2*this->size);
        // Pseudocode in the paper uses goto, so I do as well
        entry:
        Entry ent = entries[j].load();
        if (ent.cycle < cycle(t) && 
        ent.index == F_INDEX &&
        (ent.is_safe == 1 || head->load() <= t)){
            Entry new_entry = {cycle(t), 1, index};
            if (!entries[j].compare_exchange_strong(ent, new_entry)){
                goto entry;
            }
            if (threshold->load() != 3*this->size - 1){
                threshold->store(3*this->size - 1);
            }
            return;
        }
    }
}

int SCQ::deq(){
    // Checks if queue is empty
    if (threshold->load() < 0){
        return F_INDEX;
    }

    while (true){
        int h = head->fetch_add(1);
        int j = h % (2*this->size);
        Entry ent = entries[j].load();
        if (ent.cycle == cycle(h)){

        }
    }
}