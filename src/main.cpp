#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <omp.h>
#include "LockQueue.h"
#include "SCQ.h"
#include "Queue.h"
#include "main.h"

void test_enqueue(Queue * q, int id, int enq_cnt){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < enq_cnt; i++){
        bool success = q->enq(i);
        if (success){
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Successfully enqueued " << i << std::endl;
            }        
        }
        else{
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << i << std::endl;
            }        
        }
    }
}

void test_dequeue(Queue * q, int id, int deq_cnt){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < deq_cnt; i ++){
        int error_code;
        int ret = q->deq(&error_code);
        if (ret < 0){
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
            }        
        }
        else{
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Successfully dequeued " << ret << std::endl;
            }        
        }
    }
}

void test_queue(Queue * q, int id, int elements){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < elements; i ++){
        bool success = q->enq(i);
        if (success){
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Successfully enqueued " << i << std::endl;
            }        
        }
        else{
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << i << std::endl;
            }        
        }
    }

    for (int i = 0; i < elements; i ++){
        int error_code;
        int ret = q->deq(&error_code);
        if (ret < 0){
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
            }        
        }
        else{
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Successfully dequeued " << ret << std::endl;
            }    
        }
    }
}

int main(int argc, char **argv){
    
    /**
     * Usage: program_name [Queue Type [Num. of threads]]
     */
 
    int num_threads = DEFAULT_THREADS;
    int enq_cnt = 10;   //Enq_cnt per thread
    int deq_cnt = 10;   //Deq_cnt per thread
   
    int q_type = 0; // 0 ... Lock, otherwise ... SCQ

    if (argc > 1){
        int q_type = atoi(argv[1]);
    }
    if (argc > 2){
        int arg_num_threads = atoi(argv[2]);
        std::cout << "num threads: " << arg_num_threads << std::endl;
        if (arg_num_threads > 0 && arg_num_threads <= MAX_THREADS){
            num_threads = arg_num_threads;
        }
        else{
            if(!BENCHMARK){
                std::cout << "Invalid number of threads, using default." << std::endl;
            }       
        }
    }
    
    if(USE_OPENMP){
        num_threads = omp_get_max_threads();
    }
    
    //LockQueue q = LockQueue(8);
    LockQueue lq(enq_cnt*num_threads);
    SCQ scq(enq_cnt*num_threads);

    Queue * q;

    if (q_type == 0){
        q = &lq;
    }
    else{
        q = &scq;
    }
     
    if(!BENCHMARK){//
        std::cout << "Created Queue\n";
    }   
    
    std::vector<std::thread> threads;

    // Start time measurement
    auto start = std::chrono::system_clock::now();

    // Start enqueue threads
    if(!USE_OPENMP){
        for(int i = 0; i < num_threads; i++){
            threads.push_back(std::thread(test_enqueue, q, i, enq_cnt));
        }
    }
    else{
        #pragma omp parallel 
        {
            int id = omp_get_thread_num();
            test_enqueue(q, id, enq_cnt);
        }
    }
    //End Time
    auto time_enq = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();


    // Start time measurement
    start = std::chrono::system_clock::now();

    // Start dequeue threads
    if(!USE_OPENMP){
        for(int i = 0; i < num_threads; i++){
            threads.push_back(std::thread(test_dequeue, q, i, deq_cnt));
        }
    }
    else{
        #pragma omp parallel 
        {
            int id = omp_get_thread_num();
            test_dequeue(q, id, enq_cnt);
        }
    }
    // End timer
    auto time_deq = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();

    if(!USE_OPENMP){
        // Join threads
        if(!BENCHMARK){
            std::cout << "Join Threads after Enqueue Test\n";
        }
        for(auto& thread : threads){
            if (thread.joinable()){
                thread.join();
            }
        }
    }
    
    if(!BENCHMARK){
        std::cout << "Enq Time: " << time_enq << "ms" << "\nDeq Time: " << time_deq << "ms" << "Total Time: " << time_enq + time_deq << "ms\n";
    }    
    else{
        //Print in csv format
        //Benchmark format: Num_threads;Enq_cnt;Deq_cnt;Enq_time[ms];Deq_time[ms]
        std::cout << num_threads << ";" << enq_cnt << ";" << deq_cnt << ";" << time_enq << ";" << time_deq << ";" << std::endl;
    }
    

    return 0;
}