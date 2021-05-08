#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "LockQueue.h"
#include "main.h"

void test_queue(LockQueue * q, int id){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < 10; i ++){
        bool success = q->enq(i);
        if (success){
            std::cout << "Thread " << my_id << ": Successfully enqueued " << i << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << i << std::endl;
        }
        // Sleep a bit so we can really see concurrent execution
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (int i = 0; i < 10; i ++){
        int error_code;
        int ret = q->deq(&error_code);
        if (error_code < 0){
            std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Successfully dequeued " << ret << std::endl;
        }
        // Sleep a bit so we can really see concurrent execution
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main(int argc, char **argv){
    
    LockQueue q = LockQueue(8);
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

    // Run threads
    std::vector<std::thread> threads;
    for(int i = 0; i < num_threads; ++i){
        threads.push_back(std::thread(test_queue, &q, i));
    }

    // Join threads
    for(auto& thread : threads){
        thread.join();
    }

    return 0;
}