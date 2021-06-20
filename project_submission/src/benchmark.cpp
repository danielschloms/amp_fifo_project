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
    }
}


int main(int argc, char **argv){
    
    /**
     * Usage: program_name [Queue Type [Num. of threads]]
     */
    int num_threads = 1;//DEFAULT_THREADS;
    int total_cnt = 1000000;
    int enq_cnt = 0;   //Enq_cnt
    int deq_cnt = 0;   //Deq_cnt
    

    size_t q_elements = pow(2,16); //As described in the paper
   
    int q_type = Q_TYPE; //defined in benchmark.h
    int b_type = BENCHMARK_TYPE; 


    num_threads = omp_get_max_threads();
    

    LockQueue lq(q_elements);
    DoubleLockQueue dlq(q_elements);
    FIFO_SCQ scq(q_elements);
    FIFO_NCQ ncq(q_elements);


    Queue * q;
    if (argc > 1){
        q_type = atoi(argv[1]);
    }
    if (argc > 2){
        b_type = atoi(argv[2]);
    }

    #pragma omp single
    {
        if(b_type == EMPTY_DEQ){
            deq_cnt = total_cnt;
        }
        else if(b_type == PAIRS || b_type == RANDOM_OP){
            enq_cnt = total_cnt / 2;
            deq_cnt = total_cnt / 2 + total_cnt % 2;
        }  
    }

    if (q_type == LOCK_QUEUE){
        q = &lq;
    }
    else if(q_type == DOUBLE_LOCK_QUEUE){
        q = &dlq;
    }
    

    // Start time measurement
    auto start = std::chrono::system_clock::now();
    if(q_type == LOCK_QUEUE || q_type == DOUBLE_LOCK_QUEUE){
        #pragma omp parallel 
        {
            int id = omp_get_thread_num();
            if(b_type == EMPTY_DEQ){
                auto deq_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + deq_cnt % num_threads : deq_cnt/num_threads; 
                test_dequeue(q, id, deq_cnt_thread);
            }
            else if(b_type == PAIRS){
                auto total_cnt_thread = id == num_threads - 1 ? total_cnt/num_threads + total_cnt % num_threads : total_cnt/num_threads; 
                int error_code = 0;
                for(int i = 0; i < total_cnt_thread/2; i++){
                    q->enq(i);
                    q->deq(&error_code);
                }
            }
            else if(b_type == RANDOM_OP){
                auto total_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + total_cnt % num_threads : total_cnt/num_threads; 
                for (int i = 0; i < total_cnt_thread; i++){
                    int error_code;
                    int r = rand()%2;
                    if(r == 0){
                        q->enq(id);
                    }
                    else{
                        q->deq(&error_code);
                    }
                    
                }
            }
        }
    }
    else if(q_type == SCQ){
        #pragma omp parallel
        {   
            int id = omp_get_thread_num();
            if(b_type == EMPTY_DEQ){
                auto deq_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + deq_cnt % num_threads : deq_cnt/num_threads; 
                for (int i = 0; i < deq_cnt_thread; i ++){
                    int error_code;
                    scq.deq(&error_code);
                }
            }
            else if(b_type == PAIRS){
                auto total_cnt_thread = id == num_threads - 1 ? total_cnt/num_threads + total_cnt % num_threads : total_cnt/num_threads; 
                for (int i = 0; i < total_cnt_thread/2; i++){
                    int error_code;
                    scq.enq(id);
                    scq.deq(&error_code);
                }
            }
            if(b_type == RANDOM_OP){
                auto total_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + total_cnt % num_threads : total_cnt/num_threads; 
                for (int i = 0; i < total_cnt_thread; i++){
                    int error_code;
                    int r = rand()%2;
                    if(r == 0){
                        scq.enq(id);
                    }
                    else{
                        scq.deq(&error_code);
                    }
                    
                }
            }
        }
    }
    else if(q_type == NCQ){
        #pragma omp parallel
        {   
            int id = omp_get_thread_num();
            if(b_type == EMPTY_DEQ){
                auto deq_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + deq_cnt % num_threads : deq_cnt/num_threads; 
                for (int i = 0; i < deq_cnt_thread; i ++){
                    int error_code;
                    ncq.deq(&error_code);
                }
            }
            else if(b_type == PAIRS){
                auto total_cnt_thread = id == num_threads - 1 ? total_cnt/num_threads + total_cnt % num_threads : total_cnt/num_threads; 
                for (int i = 0; i < total_cnt_thread/2; i++){
                    int error_code;
                    ncq.enq(id);
                    ncq.deq(&error_code);
                }
            }
            if(b_type == RANDOM_OP){
                auto total_cnt_thread = id == num_threads - 1 ? deq_cnt/num_threads + total_cnt % num_threads : total_cnt/num_threads; 
                for (int i = 0; i < total_cnt_thread; i++){
                    int error_code;
                    int r = rand()%2;
                    if(r == 0){
                        ncq.enq(id);
                    }
                    else{
                        ncq.deq(&error_code);
                    }
                    
                }
            }
        }
    }
    // End timer
    auto time_total = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();


   
    
    //Print in csv format
    //Benchmark format: Q_type;Num_threads;Enq_cnt;Deq_cnt;;Total_time[ms]
    if(b_type != THROUGHPUT){
        std::cout << q_type << ";" << num_threads << ";" << enq_cnt << ";" << deq_cnt <<  ";" << time_total << ";" << std::endl;
    }
    else{

    }
    
    
    

    return 0;
}