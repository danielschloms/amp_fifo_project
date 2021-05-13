#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "LockQueue.h"
#include "LockQueue.h"
#include "main.h"

void test_enqueue(LockQueue * q, int id, int elements){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < elements; i++){
        bool success = q->enq(i);
        /*if (success){
            std::cout << "Thread " << my_id << ": Successfully enqueued " << i << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << i << std::endl;
        }*/
    }
}

void test_dequeue(LockQueue * q, int id, int elements){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < elements; i ++){
        int error_code;
        int ret = q->deq(&error_code);
        /*if (ret < 0){
            std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Successfully dequeued " << ret << std::endl;
        }*/
    }
}

void test_queue(LockQueue * q, int id, int elements){
    // Thread ID
    // std::thread::id my_id = std::this_thread::get_id();
    // Don't use actual thread ID, just use the thread's index
    int my_id = id;

    for (int i = 0; i < elements; i ++){
        bool success = q->enq(i);
        /*if (success){
            std::cout << "Thread " << my_id << ": Successfully enqueued " << i << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Queue full, didn't enqueue " << i << std::endl;
        }*/
    }

    for (int i = 0; i < elements; i ++){
        int error_code;
        int ret = q->deq(&error_code);
        /*if (ret < 0){
            std::cout << "Thread " << my_id << ": Queue empty, nothing dequeued" << std::endl;
        }
        else{
            std::cout << "Thread " << my_id << ": Successfully dequeued " << ret << std::endl;
        }*/
    }
}

int main(int argc, char **argv){
    
 
    int num_threads = DEFAULT_THREADS;
    int elements = 10000000;

    if (argc > 1){
        int arg_num_threads = atoi(argv[1]);
        if (arg_num_threads > 0 && arg_num_threads <= MAX_THREADS){
            num_threads = arg_num_threads;
        }
        else{
            std::cout << "Invalid number of threads, using default." << std::endl;
        }
    }
    
    //LockQueue q = LockQueue(8);
    LockQueue q = LockQueue(elements*num_threads);
    std::cout << "Created Queue\n";

    std::vector<std::thread> threads;

    // Start time measurement
    auto start = std::chrono::system_clock::now();

    // Start enqueue threads
    for(int i = 0; i < num_threads; i++){
        threads.push_back(std::thread(test_enqueue, &q, i, elements));
    }

    // Start dequeue threads
    for(int i = 0; i < num_threads; i++){
        threads.push_back(std::thread(test_dequeue, &q, i, 10));
    }

    // Join threads
    for(auto& thread : threads){
        if (thread.joinable()){
            thread.join();
        }
    }
    
    std::cout << "Join Threads after Enqueue Test\n";

    // End timer
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::system_clock::now()-start).count();
    std::cout << "Elapsed time: " << elapsed << " ms\n";

    return 0;
}