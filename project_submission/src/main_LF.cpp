#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <omp.h>
#include "FIFO_SCQ_Queue.h"
#include "FIFO_NCQ_Queue.h"
#include "main_LF.h"
#include <cassert>

bool terminate_enq = false;
bool terminate_deq = false;

void enq_loop(FIFO_SCQ * q, int id, size_t *ctr_succ, size_t *ctr_unsucc, size_t cache_offset){

    while (!terminate_enq){
        bool success = q->enq(id);
        if (success){
            ctr_succ[id*cache_offset]++;
        }
        else{
            ctr_unsucc[id*cache_offset]++;   
        }
    }
}

void deq_loop(FIFO_SCQ * q, int id, size_t *ctr_succ, size_t *ctr_unsucc, size_t cache_offset){
    while (!terminate_deq){
        int error_code = 1;
        int ret = q->deq(&error_code);
        if (error_code < 0){
            ctr_unsucc[id*cache_offset]++;
        }
        else{
            ctr_succ[id*cache_offset]++;
        }
    }
}

int main(int argc, char **argv){
 
    int num_threads;
    int time = 5;  // in secs
    int num_deq = 30;
    int num_enq = 30;

    // Cache line size 64 byte
    size_t cache_offset = 64 / sizeof(size_t);

    // Write local counters into a shared array, but make sure each thread writes on a different cache line
    size_t *ctr_succ = (size_t*)malloc((num_enq+num_deq)*cache_offset*sizeof(size_t));
    size_t *ctr_unsucc = (size_t*)malloc((num_enq+num_deq)*cache_offset*sizeof(size_t));

    size_t q_elements = 65536;
    size_t sq_elements = 128;

    num_threads = omp_get_max_threads();

    FIFO_SCQ q(q_elements);
    FIFO_SCQ sq(sq_elements);

    std::cout << "Sequential enqueue and dequeue of " << 2*sq_elements << " elements\n";
    std::cout << "Queue size: " << sq_elements << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    for (int i = 0; i<2*sq_elements; i++){
        bool s = sq.enq(i);
        if (s){
            std::cout << "Enqueued: " << i << std::endl;
        }
        else{
            std::cout << "Did not enqueue: " << i << ", queue full\n";
        }
    }
    std::cout << "---------------------------------------" << std::endl;
    for (int i = 0; i<2*sq_elements; i++){
        int ec = 1;
        int val = sq.deq(&ec);
        if (ec < 0){
            std::cout << "Nothing dequeued, queue empty\n";
        }
        else{
            std::cout << "Dequeued: " << val << std::endl;
        }
        
    }
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "End sequential test\n";
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "Concurrent test: " << num_enq << " enqueuers, " << num_deq << " dequeuers\n";
    std::cout << "Running for " << time << " seconds\n";
    std::cout << "Queue size: " << q_elements << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    #pragma omp parallel num_threads(num_enq + num_deq + 1)
    {
        int id = omp_get_thread_num();
        if (id < num_enq){
            enq_loop(&q, id, ctr_succ, ctr_unsucc, cache_offset);
        }
        else if (id < num_enq + num_deq) {
            deq_loop(&q, id, ctr_succ, ctr_unsucc, cache_offset);
        }
        else{
            std::this_thread::sleep_for(std::chrono::seconds(time));
            terminate_enq = true;
            terminate_deq = true;  
            std::cout << "Time is up, terminating enqueuers and dequeuers\n";
            std::cout << "---------------------------------------" << std::endl;
        }
    }

    size_t cuml_enq_succ = 0;
    size_t cuml_enq_unsucc = 0;
    size_t cuml_deq_succ = 0;
    size_t cuml_deq_unsucc = 0;
    size_t leftover = 0;
    
    std::cout << "Dequeuing left over elements\n";
    std::cout << "---------------------------------------" << std::endl;
    for (int i = 0; i < 10*q_elements; i++){
        int ec = 1;
        q.deq(&ec);
        if (ec >= 0){
            leftover++;
        }
    }

    bool err = cuml_deq_succ + leftover == cuml_enq_succ;

    for(size_t i = 0; i < num_enq; i++){
        cuml_enq_succ += ctr_succ[i*cache_offset];
        cuml_enq_unsucc += ctr_unsucc[i*cache_offset];
    }
    for(size_t i = num_enq; i < num_enq + num_deq; i++){
        cuml_deq_succ += ctr_succ[i*cache_offset];
        cuml_deq_unsucc += ctr_unsucc[i*cache_offset];
    }

    std::cout << "Max. OMP threads: " << num_threads << std::endl;
    std::cout << "Specified number of enqueuers: " << num_enq << std::endl;
    std::cout << "Specified number of dequeuers: " << num_enq << std::endl;
    std::cout << "Queue type: Lock-Free Queue" << std::endl;
    std::cout << "Running time: " << time << " seconds\n";
    std::cout << "Successful enqueues: " << cuml_enq_succ << std::endl;
    std::cout << "Unsuccesful enqueues: " << cuml_enq_unsucc << std::endl;
    std::cout << "Successful dequeues: " << cuml_deq_succ << std::endl;
    std::cout << "Unsuccesful dequeues: " << cuml_deq_unsucc << std::endl;
    std::cout << "Elements left in the queue: " << leftover << std::endl;
    if (err) {
        std::cout << "Last index error occured (see report)\n";
    }
    else{
        std::cout << "Last index error did NOT occur (see report)\n";
    }
    std::cout << "---------------------------------------" << std::endl;
    
    return 0;
}