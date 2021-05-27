#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <omp.h>
#include "LockQueue.h"
#include "SCQ.h"
#include "Queue.h"
#include "main_alt.h"

std::atomic<size_t> enq_succ_count(0);
std::atomic<size_t> enq_unsucc_count(0);
std::atomic<size_t> deq_succ_count(0);
std::atomic<size_t> deq_unsucc_count(0);
bool terminate_enq = false;
bool terminate_deq = false;

void enq_loop(Queue * q, int id){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    while (!terminate_enq){
        bool success = q->enq(my_id);
        if (success){
            enq_succ_count++;
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Successfully enqueued " << my_id << std::endl;
            }        
        }
        else{
            enq_unsucc_count++;
            if(!BENCHMARK){
                std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << my_id << std::endl;
            }        
        }
    }
    std::cout << "Enqueuer " << id << " done\n";
}

void deq_loop(Queue * q, int id){
    int my_id = id;
    while (!terminate_deq){
        int error_code;
        int ret = q->deq(&error_code);
        if (ret < 0 || error_code < 0){
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

int main(int argc, char **argv){
    
    /**
     * Usage: main [Queue Type [Num. of threads [Time]]]
     */
 
    int num_threads = DEFAULT_THREADS;
    int time = 10;  // in secs
   
    int q_type = 0; // 0 ... Lock, otherwise ... SCQ
    size_t q_elements = 1024;

    if (argc > 1){
        q_type = atoi(argv[1]);
        printf("%s\n", argv[1]);
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
    if (argc > 3){
        time = atoi(argv[3]);
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
    //auto start = std::chrono::system_clock::now();

    // Start enqueue threads
    if(!USE_OPENMP){

        // Half of threads are enqueuers, half dequeuers
        // Enqueuers first, because they have to be terminated first so they don't get stuck
        for(int i = 0; i < num_threads / 2; ++i){
            threads.push_back(std::thread(enq_loop, q, i));
        }
        for(int i = num_threads / 2; i < num_threads; ++i){
            threads.push_back(std::thread(deq_loop, q, i));
        }
    }
    else{
        #pragma omp parallel 
        {
            int id = omp_get_thread_num();
            //test_enqueue(q, id);
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(time));
    //End Time
    //auto time_enq = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();

    if(!USE_OPENMP){
        // Join threads
        if(!BENCHMARK){
            std::cout << "Join Threads after Enqueue Test\n";
        }
        terminate_enq = true;
        std::cout << "TERMINATE ENQ\n";
        terminate_deq = true;
        for(int i = 0; i < num_threads / 2; ++i){
            if (threads[i].joinable()){
                threads[i].join();
                std::cout << "Join enq: " << i << std::endl;
            }
        }
        terminate_deq = true;
        std::cout << "TERMINATE DEQ\n";
        for(int i = num_threads / 2; i < num_threads; ++i){
            if (threads[i].joinable()){
                threads[i].join();
            }
        }
    }
    
    if(!BENCHMARK){
        //std::cout << "Enq Time: " << time_enq << "ms" << "\nDeq Time: " << time_deq << "ms" << "Total Time: " << time_enq + time_deq << "ms\n";
    }    
    else{
        //Print in csv format
        //Benchmark format: Num_threads;Enq_cnt;Deq_cnt;Enq_time[ms];Deq_time[ms]
        //std::cout << num_threads << ";" << enq_cnt << ";" << deq_cnt << ";" << time_enq << ";" << time_deq << ";" << std::endl;
        std::cout << "Threads: " << num_threads << std::endl;
        std::cout << "Queue type: " << (q_type == 0 ? "Locking queue" : "Lock-free queue") << std::endl;
        std::cout << "Running time before joining threads: " << time << " seconds\n";
        std::cout << "Successful enqueue operations: " << enq_succ_count.load() << std::endl;
        std::cout << "Unuccessful enqueue operations: " << enq_unsucc_count.load() << std::endl;
        std::cout << "Successful dequeue operations: " << deq_succ_count.load() << std::endl;
        std::cout << "Unsuccessful dequeue operations: " << deq_unsucc_count.load() << std::endl;
        std::cout << "-----------------------------------------------\n";
    }
    

    return 0;
}