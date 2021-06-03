#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <omp.h>
#include "LockQueue.h"
#include "SCQ.h"
#include "Queue.h"
#include "main.h"

std::atomic<size_t> enq_succ_count(0);
std::atomic<size_t> enq_unsucc_count(0);
std::atomic<size_t> deq_succ_count(0);
std::atomic<size_t> deq_unsucc_count(0);

void test_enqueue(Queue * q, int id, int enq_cnt){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < enq_cnt; i++){
        bool success = q->enq((i + 1) * id);
        if (success){
            enq_succ_count.fetch_add(1);
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Successfully enqueued " << i << std::endl;
            }        
        }
        else{
            enq_unsucc_count.fetch_add(1);
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
        if (error_code < 0){
            deq_unsucc_count.fetch_add(1);
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
            }        
        }
        else{
            deq_succ_count.fetch_add(1);
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
    int num_threads = 1;//DEFAULT_THREADS;
    int enq_cnt = 16;   //Enq_cnt per thread
    int deq_cnt = 30;   //Deq_cnt per thread
    size_t q_elements = 8;
   
    int q_type = 1; // 0 ... Lock, otherwise ... SCQ

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
    LockQueue lq(q_elements);
    SCQ scq(q_elements);

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
            //Enqueue count per thread. If not divisible, then last thread is responsible for remaining enqueues
            auto enq_cnt_thread = i == num_threads - 1 ? enq_cnt/num_threads + enq_cnt % num_threads : enq_cnt/num_threads; 
            threads.push_back(std::thread(test_enqueue, q, i, enq_cnt_thread));
        }
    }
    else{
        #pragma omp parallel 
        {
            int id = omp_get_thread_num();
            //Dequeue count per thread. If not divisible, then last thread is responsible for remaining dequeues
            auto enq_cnt_thread = id == num_threads - 1 ? enq_cnt/num_threads + enq_cnt % num_threads : enq_cnt/num_threads; 
            
            test_enqueue(q, id, enq_cnt_thread);
        }
    }
    //End Time
    auto time_enq = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();

    for(int i = 0; i < threads.size(); i++){
            if (threads[i].joinable()){
                threads[i].join();
                //std::cout << "Join enq: " << i << std::endl;
            }
    }

    // Start time measurement
    start = std::chrono::system_clock::now();

    // Start dequeue threads
    if(!USE_OPENMP){
        for(int i = 0; i < num_threads; i++){
            auto deq_cnt_thread = i == num_threads - 1 ? deq_cnt/num_threads + deq_cnt % num_threads : deq_cnt/num_threads;
            threads.push_back(std::thread(test_dequeue, q, i, deq_cnt_thread));
        }
    }
    else{
        #pragma omp parallel 
        {
            int id = omp_get_thread_num();
            auto deq_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + deq_cnt % num_threads : deq_cnt/num_threads; 
            test_dequeue(q, id, deq_cnt_thread);
        }
    }
    for(int i = 0; i < threads.size(); i++){
            if (threads[i].joinable()){
                threads[i].join();
                //std::cout << "Join enq: " << i << std::endl;
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
        std::cout << "Threads: " << num_threads << std::endl;
        std::cout << "Queue type: " << (q_type == 0 ? "Locking queue" : "Lock-free queue") << std::endl;
        std::cout << "Running time before joining threads: " << time << " seconds\n";
        std::cout << "Successful enqueue operations: " << enq_succ_count.load() << std::endl;
        std::cout << "Unuccessful enqueue operations: " << enq_unsucc_count.load() << std::endl;
        std::cout << "Successful dequeue operations: " << deq_succ_count.load() << std::endl;
        std::cout << "Unsuccessful dequeue operations: " << deq_unsucc_count.load() << std::endl;
        std::cout << "-----------------------------------------------\n";
    }    
    else{
        //Print in csv format
        //Benchmark format: Num_threads;Enq_cnt;Deq_cnt;Enq_time[ms];Deq_time[ms]
        std::cout << num_threads << ";" << enq_cnt << ";" << deq_cnt << ";" << time_enq << ";" << time_deq << ";" << std::endl;
    }
    

    return 0;
}