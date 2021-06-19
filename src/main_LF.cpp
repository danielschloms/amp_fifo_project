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
std::atomic<size_t> finished_enqueuers(0);


void enq_loop(FIFO_SCQ * q, int id, size_t *ctr_succ, size_t *ctr_unsucc, size_t cache_offset){

    while (!terminate_enq){
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    int my_id = id;
    while (!terminate_deq){
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
 
    int num_threads = 21;
    int time = 3;  // in secs
    int num_deq = 15;
    int num_enq = 15;

    // Cache line size 64 byte
    size_t cache_offset = 64 / sizeof(size_t);

    // Write local counters into a shared array, but make sure each thread writes on a different cache line
    size_t *ctr_succ = (size_t*)malloc((num_enq+num_deq)*cache_offset*sizeof(size_t));
    size_t *ctr_unsucc = (size_t*)malloc((num_enq+num_deq)*cache_offset*sizeof(size_t));

    size_t q_elements = 32;

    num_threads = omp_get_max_threads();
    
    //Can use NCQ now too
    FIFO_SCQ q(q_elements);
    if(false){
    //for (int j = 0; j < 5; j++){
        for (int i = 0; i<2*q_elements; i++){
            bool s = q.enq(i);
            std::cout << (s?"Successfully ":"Unsuccessfully ") << "Enqueued: " << i << std::endl;
        }

        for (int i = 0; i<2*q_elements; i++){
            int ec = 1;
            std::cout << "Dequeued: " << q.deq(&ec) << " Error Code: " << ec << std::endl;
        }
    }
    
    #pragma omp parallel num_threads(num_enq + num_deq + 1)
    {
        int id = omp_get_thread_num();
        if (id < num_enq){
            //std::cout << "Enq " << id << std::endl;
            enq_loop(&q, id, ctr_succ, ctr_unsucc, cache_offset);
        }
        else if (id < num_enq + num_deq) {
            //std::cout << "Deq " << id << std::endl;
            deq_loop(&q, id, ctr_succ, ctr_unsucc, cache_offset);
        }
        else{
            // Timer thread, terminate enqueuers and dequeuers after specified time
            //std::cout << "Check aq beforebefore\n";
            //q.check_entries(true);
            //std::cout << "Check fq beforebefore\n";
            //q.check_entries(false);
            //std::this_thread::sleep_for(std::chrono::seconds(time));
            //std::cout << "Check aq before\n";
            //q.check_entries(true);
            //std::cout << "Check fq before\n";
            //q.check_entries(false);

            std::this_thread::sleep_for(std::chrono::seconds(time));
            terminate_enq = true;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            terminate_deq = true;
            
            
        }
    }

    size_t cuml_enq_succ = 0;
    size_t cuml_enq_unsucc = 0;
    size_t cuml_deq_succ = 0;
    size_t cuml_deq_unsucc = 0;
    size_t leftover = 0;

    // Make sure all elements are dequeued
    bool leave = false;
    bool err = false;
    //while (!leave){
    for (int i = 0; i < 1000*q_elements; i++){
        //q.refresh();
        leave = true;
        int ec = 1;
        q.deq(&ec);
        //std::cout << "Dequeued: " << q.deq(&ec) << " Error Code: " << ec << std::endl;
        if (ec >= 0){
            leftover++;
        }
    }

    for (int i = 0; i < 32; i++){

        if (q.num_thread[i].load() != 0){
            err = true;
            std::cout << "Error element " << i << " val: " << q.num_thread[i].load() << std::endl;
        }
    }

    if (err){
        
        /*for (int i = 0; i < 64; i++){
            bool s = q.enq(i);
            std::cout << "Enqueued " << i << (s?" successfully":" unsuccessfully") << std::endl;
        }*/

        printf("Check aq\n");
        q.check_entries(true);
        printf("Check fq\n");
        q.check_entries(false);

        q.refresh();
        int ec = 1;
        int e = q.deq(&ec);
        std::cout << "Error deq " << e << " with error code " << ec << std::endl;
    }


    for(size_t i = 0; i < num_enq; i++){
        cuml_enq_succ += ctr_succ[i*cache_offset];
        cuml_enq_unsucc += ctr_unsucc[i*cache_offset];
    }
    for(size_t i = num_enq; i < num_enq + num_deq; i++){
        cuml_deq_succ += ctr_succ[i*cache_offset];
        cuml_deq_unsucc += ctr_unsucc[i*cache_offset];
    }

    //assert(leftover == cuml_enq_succ - cuml_deq_succ);

    if(!BENCHMARK){
        std::cout << "Threads: " << num_threads << std::endl;
        std::cout << "Queue type: Lock-Free Queue" << std::endl;
        std::cout << "Running time before joining threads: " << time << " seconds\n";
        std::cout << "Enq Succ: " << cuml_enq_succ << std::endl;
        std::cout << "Enq Unsucc: " << cuml_enq_unsucc << std::endl;
        std::cout << "Deq Succ: " << cuml_deq_succ << std::endl;
        std::cout << "Deq Unsucc: " << cuml_deq_unsucc << std::endl;
        std::cout << "Left over: " << leftover << std::endl;
        std::cout << "-----------------------" << std::endl;
    }
    else{
        //Print in csv format
        //Benchmark format: Num_threads;Enq_cnt;Deq_cnt;Enq_time[ms];Deq_time[ms]
        //std::cout << num_threads << ";" << enq_cnt << ";" << deq_cnt << ";" << time_enq << ";" << time_deq << ";" << std::endl;
    }
    
    return 0;
}