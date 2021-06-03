/**
 * Scalable Implementation of the Circular Queue (SCQ)
 */

#include "SCQ_bit.h"
#include <iostream>
#include <atomic>
#include <cstddef>

SCQ::SCQ(int capacity) : 
    size(capacity),
    head(new std::atomic<size_t>(2*capacity)),
    tail(new std::atomic<size_t>(2*capacity)),
    threshold(new std::atomic<int>(-1)),
    F_INDEX(2*capacity - 1)
{
    for (size_t i = 0; i < 2*capacity; i++){
        Entry e;
        e.entr = new std::atomic<uint64_t>;
        e.entr->fetch_or(2*size - 1);
        for (size_t i = 0; i < 16; i++)
        {
            e.pad[i] = i;
        }
        
        entries_lli.push_back(e);
    }
    
    // TODO: Are the entries initialized properly?
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

int SCQ::cycle(int x){
    return x / (size);
}

bool SCQ::entry_empty(int j){
    return entries_lli[j].entr->load() & F_INDEX == F_INDEX;
}

bool SCQ::enq(uint64_t index){
    /*
    if (index == F_INDEX){
        std::cout << "Value can't be magic number" << std::endl;
        return false;
    }
    */
    index ^= (size - 1);
    size_t loop_cnt = 0;
    size_t loop_cnt_2 = 0;
    while (true){
        //printf("asdf\n");
        loop_cnt++;
        loop_cnt_2++;
        uint64_t tc = cycle(tail->load()+1);
        uint64_t hc = cycle(head->load());
        if (loop_cnt_2 > 10000000){
            printf("What\n");
            loop_cnt_2 = 0;
        }
        if (tc > 1 + hc) continue;

        size_t t = tail->fetch_add(1); 
        // In the pseudocode, cache_remap is used to reduce false sharing
        // j = cache_remap(T % (2*n))
        // TODO: Check if padding in Entry struct works
        size_t j = t % (size);

        
        
        uint64_t ent = entries_lli[j].entr->load();
        entry_load_enq:
        // Pseudocode in the paper uses goto, so I do as well
        
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
        else if (loop_cnt > 10000000){
            std::cout << "Issue with Entry " << j << std::endl;
            uint64_t issue_ent = entries_lli[j].entr->load();
            uint64_t issue_cycle = issue_ent >> (size - 1);
            uint64_t issue_issafe = issue_ent & size;
            uint64_t issue_index = issue_ent & (size - 1);
            std::cout << "Cycle: " << issue_cycle << std::endl;
            std::cout << "IsSafe: " << issue_issafe << std::endl;
            std::cout << "Index: " << issue_index << std::endl;
            std::cout << "Threshold: " << threshold->load() << std::endl;
            std::cout << "----------------------------------------\n";
            loop_cnt = 0;
        }
    }
}

int SCQ::deq(int * error_code){
    // Checks if queue is empty
    *error_code = 1;
    if (threshold->load() < 0){
        *error_code = -1;
        is_empty = true;
        return ~0;
    }

    while (true){
        size_t h = head->fetch_add(1);
        size_t j = h % (size);

        
        uint64_t ent = entries_lli[j].entr->load();
        entry_load_deq:
        uint64_t ent_cycle = ent | (2*size - 1);
        uint64_t h_cycle = (h << 1) | (2*size - 1);

        if (ent_cycle == h_cycle){
            uint64_t current = entries_lli[j].entr->load();
            entries_lli[j].entr->fetch_or(size - 1);
            return ent & (size - 1);
        }

        // Cycle(ent), 0, Index(Ent)
        uint64_t new_ent = ent & (~size);

        if (ent | F_INDEX == ent){
            // Cycle(h), Ent.IsSafe, F_INDEX
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
            *error_code = -1;
            return ~0;
        }

        if (threshold->fetch_sub(1) <= 0){
            *error_code = -1;
            is_empty = true;
            return ~0;
        }
    }
}