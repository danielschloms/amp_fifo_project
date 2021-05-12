#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "LockQueue.h"
#include "SCQ.h"
#include "main.h"

void test_enqueue(SCQ * q, int id, int elements){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < elements; i ++){
        bool success = q->enq(i);
        if (success){
            std::cout << "Thread " << my_id << ": Successfully enqueued " << i << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << i << std::endl;
        }
    }
}

void test_dequeue(SCQ * q, int id, int elements){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < elements; i ++){
        int error_code;
        int ret = q->deq(&error_code);
        if (error_code < 0){
            std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Successfully dequeued " << ret << std::endl;
        }
    }
}

void test_queue(SCQ * q, int id, int elements){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < elements; i ++){
        bool success = q->enq(i);
        if (success){
            std::cout << "Thread " << my_id << ": Successfully enqueued " << i << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << i << std::endl;
        }
    }

    for (int i = 0; i < elements; i ++){
        int error_code;
        int ret = q->deq(&error_code);
        if (error_code < 0){
            std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Successfully dequeued " << ret << std::endl;
        }
    }
}

int main(int argc, char **argv){
    
    //LockQueue q = LockQueue(8);
    SCQ q = SCQ(8);
    std::cout << "Created Queue\n";
    int num_threads = DEFAULT_THREADS;

    if (argc > 1){
        int arg_num_threads = atoi(argv[1]);
        if (arg_num_threads > 0 && arg_num_threads <= MAX_THREADS){
            num_threads = arg_num_threads;
        }
        else{
            std::cout << "Invalid number of threads, using default." << std::endl;
        }
    }    

    std::vector<std::thread> threads;

    // Start time measurement
    auto start = std::chrono::system_clock::now();

    // Start threads
    for(int i = 0; i < num_threads; ++i){
        threads.push_back(std::thread(test_enqueue, &q, i, 10));
    }

    // Join threads
    for(auto& thread : threads){
        thread.join();
    }

    // End timer
    auto end = std::chrono::system_clock::now();
    auto elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << '\n';

    return 0;
}