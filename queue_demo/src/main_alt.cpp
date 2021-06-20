#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <omp.h>
#include "LockQueue.h"
#include "DoubleLockQueue.h"
#include "NCQ.h"
#include "SCQ.h"
#include "Queue.h"
#include "main_alt.h"

std::atomic<size_t> enq_succ_count(0);
std::atomic<size_t> enq_unsucc_count(0);
std::atomic<size_t> deq_succ_count(0);
std::atomic<size_t> deq_unsucc_count(0);
bool terminate_enq = false;
bool terminate_deq = false;
std::atomic<size_t> finished_enqueuers(0);

void enq_loop(Queue * q, int id, size_t *ctr_succ, size_t *ctr_unsucc, size_t cache_offset){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    while (!terminate_enq){
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        bool success = q->enq(my_id);
        if (success){
            //enq_succ_count.fetch_add(1);
            ctr_succ[id*cache_offset]++;
            //if(!BENCHMARK){
            //    std::cout << "Thread " << my_id << ": Successfully enqueued " << my_id << std::endl;
            //}        
        }
        else{
            //enq_unsucc_count.fetch_add(1);
            ctr_unsucc[id*cache_offset]++;
            //if(!BENCHMARK){
            //    std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << my_id << std::endl;
            //}        
        }
    }
    finished_enqueuers.fetch_add(1);
}

void deq_loop(Queue * q, int id, size_t *ctr_succ, size_t *ctr_unsucc, size_t cache_offset){
    int my_id = id;
    while (!terminate_deq){
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        int error_code;
        int ret = q->deq(&error_code);
        if (error_code < 0){
            //deq_unsucc_count.fetch_add(1);
            ctr_unsucc[id*cache_offset]++;
            //if(!BENCHMARK){
            //    std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
            //}        
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else{
            //deq_succ_count.fetch_add(1);
            ctr_succ[id*cache_offset]++;
            //if(!BENCHMARK){
            //    std::cout << "Thread " << my_id << ": Successfully dequeued " << ret << std::endl;
            //}    
        }
    }
}

int main(int argc, char **argv){
    
    /**
     * Usage: main [Queue Type [Num. of threads [Time]]]
     */
 
    int num_threads = 21;
    int time = 10;  // in secs
    int num_deq = 15;
    int num_enq = 15;

    // Cache line size 64 byte
    size_t cache_offset = 64 / sizeof(size_t);

    // Write local counters into a shared array, but make sure each thread writes on a different cache line
    size_t *ctr_succ = (size_t*)malloc((num_enq+num_deq)*cache_offset*sizeof(size_t));
    size_t *ctr_unsucc = (size_t*)malloc((num_enq+num_deq)*cache_offset*sizeof(size_t));

    int q_type = 1; // 0 ... Lock, otherwise ... SCQ
    size_t q_elements = 8;

    /*if (argc > 1){
        q_type = atoi(argv[1]);
    }
    if (argc > 2){
        int arg_num_threads = atoi(argv[2]);
        if (arg_num_threads > 0 && arg_num_threads <= MAX_THREADS){
            num_threads = arg_num_threads;
        }
        else{
            if(!BENCHMARK){
                std::cout << "Invalid number of threads, using default." << std::endl;
            }       
        }
    }
    if (argc > 3){
        time = atoi(argv[3]);
    }*/
    
    if(USE_OPENMP){
        num_threads = omp_get_max_threads();
    }
    
    //LockQueue q = LockQueue(8);
    LockQueue lq(q_elements);
    SCQ scq(q_elements);
    NCQ ncq(q_elements);
    DoubleLockQueue dlq(q_elements);

    Queue * q;
    q_type = 3;
    if (q_type == 0){
        q = &lq;
    }
    else if(q_type == 1){
        q = &scq;
    }
    else if(q_type == 2){
        q = &ncq;
    }
    else if(q_type == 3){
        q = &dlq;
    }

    std::vector<std::thread> enq_threads;
    std::vector<std::thread> deq_threads;
    std::vector<std::thread> threads;

    // Start time measurement
    //auto start = std::chrono::system_clock::now();

    // Start enqueue threads
    if(!USE_OPENMP){

        // Half of threads are enqueuers, half dequeuers
        // Enqueuers first, because they have to be terminated first so they don't get stuck
        /*for(int i = 0; i < num_threads / 2; i++){
            enq_threads.push_back(std::thread(enq_loop, q, i));
        }
        for(int i = 0; i < num_threads / 2; i++){
            deq_threads.push_back(std::thread(deq_loop, q, (num_threads/2)+i));
        }*/
    }
    else{
        #pragma omp parallel num_threads(num_enq + num_deq + 1)
        {
            int id = omp_get_thread_num();
            if (id < num_enq){
                //std::cout << "Enq " << id << std::endl;
                enq_loop(q, id, ctr_succ, ctr_unsucc, cache_offset);
            }
            else if (id < num_enq + num_deq) {
                //std::cout << "Deq " << id << std::endl;
                deq_loop(q, id, ctr_succ, ctr_unsucc, cache_offset);
            }
            else{
                // Timer thread, terminate enqueuers and dequeuers after specified time
                std::this_thread::sleep_for(std::chrono::seconds(time));
                terminate_enq = true;
                q->kill();
                std::cout << "Time over\n";
                // Wait until enqueuers are returned, otherwise they can get stuck
                //while (finished_enqueuers.load() < num_enq);
                terminate_deq = true;
            }
        }
    }
    //End Time
    //auto time_enq = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();

    if(!USE_OPENMP){
        terminate_enq = true;
        for(int i = 0; i < enq_threads.size(); i++){
            if (enq_threads[i].joinable()){
                enq_threads[i].join();
            }
        }
        terminate_deq = true;
        for(int i = 0; i < deq_threads.size(); i++){
            if (deq_threads[i].joinable()){
                deq_threads[i].join();
            }
        }
    }

    size_t cuml_enq_succ = 0;
    size_t cuml_enq_unsucc = 0;
    size_t cuml_deq_succ = 0;
    size_t cuml_deq_unsucc = 0;
    for(size_t i = 0; i < num_enq; i++){
        cuml_enq_succ += ctr_succ[i*cache_offset];
        cuml_enq_unsucc += ctr_unsucc[i*cache_offset];
    }
    for(size_t i = num_enq; i < num_enq + num_deq; i++){
        cuml_deq_succ += ctr_succ[i*cache_offset];
        cuml_deq_unsucc += ctr_unsucc[i*cache_offset];
    }

    

    if(!BENCHMARK){
        std::string queue;
        switch (q_type)
        {
            case 0:
                queue =  "Lock ";
                break;
            case 1:
                queue = "SCQ ";
                break;
            case 2: 
                queue = "NCQ ";
                break;
            case 3: 
                queue = "Double Lock ";
                break;
            default:
                break;
        }

        std::cout << "Threads: " << num_threads << std::endl;
        std::cout << "Queue type: " << queue << "queue" << std::endl;
        std::cout << "Running time before joining threads: " << time << " seconds\n";
        std::cout << "Enq Succ: " << cuml_enq_succ << std::endl;
        std::cout << "Enq Unsucc: " << cuml_enq_unsucc << std::endl;
        std::cout << "Deq Succ: " << cuml_deq_succ << std::endl;
        std::cout << "Deq Unsucc: " << cuml_deq_unsucc << std::endl;
        std::cout << "-----------------------" << std::endl;

        int nonempty_cnt = 0;
        for (size_t i = 0; i < 2*q_elements; i++){
        if (!scq.entry_empty(i)){
            //scq.print_entry(i);
            nonempty_cnt++;
        }
    }
    std::cout << "Nonempty: " << nonempty_cnt << std::endl;
    }    
    else{
        //Print in csv format
        //Benchmark format: Num_threads;Enq_cnt;Deq_cnt;Enq_time[ms];Deq_time[ms]
        //std::cout << num_threads << ";" << enq_cnt << ";" << deq_cnt << ";" << time_enq << ";" << time_deq << ";" << std::endl;
    }

    

    return 0;
}