#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <omp.h>
#include <math.h> 
#include "LockQueue.h"
#include "DoubleLockQueue.h"
#include "FIFO_SCQ_Queue.h"
#include "FIFO_NCQ_Queue.h"
#include "benchmark.h"

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
    }
}

void test_dequeue(Queue * q, int id, int deq_cnt){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    //int my_id = id;

    for (int i = 0; i < deq_cnt; i ++){
        int error_code;
        int ret = q->deq(&error_code);
        /*if (error_code < 0){
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
        }*/
    }
}


int main(int argc, char **argv){
    
    /**
     * Usage: program_name [Queue Type [Num. of threads]]
     */
    int num_threads = 1;//DEFAULT_THREADS;
    int enq_cnt = 0;   //Enq_cnt
    int deq_cnt = 10000000;   //Deq_cnt
    size_t q_elements = pow(2,16); //As described in the paper
   
    int q_type = Q_TYPE; //defined in benchmark.h


    num_threads = omp_get_max_threads();
    

    LockQueue lq(q_elements);
    DoubleLockQueue dlq(q_elements);
    FIFO_SCQ scq(q_elements);
    FIFO_NCQ ncq(q_elements);


    Queue * q;
    if (argc > 1){
        q_type = atoi(argv[1]);
    }

    if (q_type == LOCK_QUEUE){
        q = &lq;
    }
    else if(q_type == DOUBLE_LOCK_QUEUE){
        q = &dlq;
    }
    
    // Start time measurement
    auto start = std::chrono::system_clock::now();


    auto time_enq = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();
    
    // Start time measurement
    start = std::chrono::system_clock::now();
    if(q_type == LOCK_QUEUE || q_type == DOUBLE_LOCK_QUEUE){
        #pragma omp parallel 
        {
            int id = omp_get_thread_num();
            auto deq_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + deq_cnt % num_threads : deq_cnt/num_threads; 
            test_dequeue(q, id, deq_cnt_thread);
        }
    }
    else if(q_type == SCQ){
        #pragma omp parallel
        {
            int id = omp_get_thread_num();
            auto deq_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + deq_cnt % num_threads : deq_cnt/num_threads; 
            for (int i = 0; i < deq_cnt; i ++){
                int error_code;
                scq.deq(&error_code);
            }
        }
    }
    else if(q_type == NCQ){
         #pragma omp parallel
        {
            int id = omp_get_thread_num();
            auto deq_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + deq_cnt % num_threads : deq_cnt/num_threads; 
            for (int i = 0; i < deq_cnt; i ++){
                int error_code;
                ncq.deq(&error_code);
            }
        }
    }
    // End timer
    auto time_deq = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();


   
    
    //Print in csv format
    //Benchmark format: Q_type;Num_threads;Enq_cnt;Deq_cnt;Enq_time[ms];Deq_time[ms]
    std::cout << q_type << ";" << num_threads << ";" << enq_cnt << ";" << deq_cnt << ";" << time_enq << ";" << time_deq << ";" << std::endl;
    
    

    return 0;
}