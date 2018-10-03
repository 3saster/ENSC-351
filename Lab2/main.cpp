#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <queue>
#include <iostream>
#include <csignal>

#include "tracelib.h"
#define LOGGING 1 //Set to one to turn on tracing

/*  CONSTANTS   */
#ifndef NUM_THREADS
    #define NUM_THREADS 1000
#endif

/*  GLOBALS */
int door(0); //intentionally non-atomic
std::atomic<bool> thread_init(false);
//Used in method 2
bool cellArray[NUM_THREADS+1] = { false };
//Used in method 3
int tickets[NUM_THREADS+1] = { 0 }; //Ensure going beyond the end hits a non-existent thread (0)
std::atomic<int*> nowServing(&tickets[0]);
std::atomic<int> lineSpot(0);

/*  MUTEXES */
std::mutex mtx;

/*  FUNCTIONS   */
//Method 1: A simple lock (in theory unnecessary if door was atomic)
void method_1(const unsigned int tid)
{
    //Wait for other threads
    while(!thread_init)
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    mtx.lock();
    trace::trace_event_start("Method 1 Increment", "Method 1", tid);
    door++;
    trace::trace_event_end(tid);
    mtx.unlock();
}

//Method 2: Essentially, thread 2 increments door, then thread 3, and so forth
void method_2(const unsigned int tid)
{
    //Wait for other threads
    while(!thread_init)
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    int true_tid = tid-2; //Match tid to cell array; i.e. thread 2 = 0 in array
    //Wait your turn
    while( !cellArray[true_tid] )
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    trace::trace_event_start("Method 2 Increment", "Method 2", tid);
    door++; //Enter
    trace::trace_event_end(tid);
    cellArray[true_tid+1] = true; //Allow next person to enter
}

//Method 3: Ticket Lock Implementation
void method_3(const unsigned int tid)
{
    //Wait for other threads
    while(!thread_init)
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    //Add ticket to ticket list
    mtx.lock();
    tickets[lineSpot++] = tid;
    mtx.unlock();

    //Wait your turn
    while( *nowServing != tid )
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    trace::trace_event_start("Method 3 Increment", "Method 3", tid);
    door++; //Enter
    trace::trace_event_end(tid);
    nowServing++; //Allow next person to go
}

/*  MAIN THREAD  */
int main(int argc, char *argv[])
{
    std::thread threadArray[NUM_THREADS];
    /*  START LOGGING   */
    #if LOGGING
        trace::trace_start("trace.json");
        auto traceEnd = [] (int i) { trace::trace_end(); exit(128+i); };
        std::signal(SIGINT, traceEnd); //^C
        std::signal(SIGABRT, traceEnd); //abort()
    #endif

    /*  METHOD 1    */
    door = 0;
    thread_init = false;
    //Create Array of Threads
    trace::trace_event_start("Thread Initialization", "Method 1");
    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i] = std::thread(method_1,i+2); //First thread created should be thread 2
    }
    trace::trace_event_end();

    std::cout << "Method 1 threads have been initialized.\n";
    trace::trace_event_start("Method 1", "Method 1");
    thread_init = true;

    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i].join();
    }
    trace::trace_event_end();

    std::cout << "There are " << door << " people inside after Method 1.\n";

    /*  METHOD 2    */
    door = 0;
    thread_init = false;
    //Create Array of Threads
    trace::trace_event_start("Thread Initialization", "Method 2");
    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i] = std::thread(method_2,i+2); //First thread created should be thread 2
    }
    trace::trace_event_end();

    std::cout << "Method 2 threads have been initialized.\n";
    trace::trace_event_start("Method 2", "Method 2");
    thread_init = true;
    //Let first guy in
    cellArray[0] = true;

    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i].join();
    }
    trace::trace_event_end();

    std::cout << "There are " << door << " people inside after Method 2.\n";

    /*  METHOD 3    */
    door = 0;
    thread_init = false;
    //Create Array of Threads
    trace::trace_event_start("Thread Initialization", "Method 3");
    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i] = std::thread(method_3,i+2); //First thread created should be thread 2
    }
    trace::trace_event_end();

    std::cout << "Method 3 threads have been initialized.\n";
    trace::trace_event_start("Method 3", "Method 3");
    thread_init = true;

    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i].join();
    }
    trace::trace_event_end();

    std::cout << "There are " << door << " people inside after Method 3.\n";

    /*  END LOGGING */
    #if LOGGING
        trace::trace_end();
    #endif

    return 0;
}
