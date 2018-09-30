#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <iostream>

#include "tracelib.h"
#define LOGGING 1 //Set to one to turn on tracing

/*  CONSTANTS   */
const int NUM_THREADS = 500;

/*  GLOBALS */
int door(0); //intentionally non-atomic
std::atomic<bool> thread_init(false);
//Used in method 2
bool cellArray[NUM_THREADS+1] = { false };

/*  MUTEXES */
std::mutex mtx;

/*  FUNCTIONS   */
//Method 1: A simple lock (in theory unnecessary since door is atomic)
void method_1(const unsigned int tid)
{
    //Wait for other threads
    while(!thread_init)
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    mtx.lock();
    trace::trace_event_start("Method 1 Increment", "Main", tid);
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

    //Wait your turn
    while( !cellArray[tid-2] )
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }

    int true_tid = tid-2; //Match tid to cell array; i.e. thread 2 = 0 in array

    trace::trace_event_start("Method 2 Increment", "Main", tid);
    door++; //Enter
    cellArray[true_tid+1] = true; //Allow next person to enter
    trace::trace_event_end(tid);
}

/*  MAIN THREAD  */
int main(int argc, char *argv[])
{
    std::thread threadArray[NUM_THREADS];

    /*  METHOD 1    */
    #if LOGGING
        trace::trace_start("method_1.json");
    #endif

    door = 0;
    thread_init = false;
    //Create Array of Threads
    trace::trace_event_start("Thread Initialization", "Init");
    trace::trace_counter("Threads Created", {"Extra Threads"}, {"0"} );
    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i] = std::thread(method_1,i+2); //First thread created should be thread 2
        trace::trace_counter("Extra Threads", {"Extra Threads"}, {std::to_string(i).c_str()} );
    }
    trace::trace_event_end();

    thread_init = true;

    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i].join();
        trace::trace_counter("Extra Threads", {"Extra Threads"}, {std::to_string(NUM_THREADS-i-1).c_str()} );
    }

    std::cout << "There are " << door << " people inside after Method 1.\n";

    #if LOGGING
        trace::trace_end();
    #endif

    /*  METHOD 2    */
    #if LOGGING
        trace::trace_start("method_2.json");
    #endif

    door = 0;
    thread_init = false
    //Create Array of Threads
    trace::trace_event_start("Thread Initialization", "Init");
    trace::trace_counter("Threads Created", {"Extra Threads"}, {"0"} );
    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i] = std::thread(method_2,i+2); //First thread created should be thread 2
        trace::trace_counter("Extra Threads", {"Extra Threads"}, {std::to_string(i).c_str()} );
    }
    trace::trace_event_end();

    thread_init = true;
    //Let first guy in
    cellArray[0] = true;

    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i].join();
        trace::trace_counter("Extra Threads", {"Extra Threads"}, {std::to_string(NUM_THREADS-i-1).c_str()} );
    }

    std::cout << "There are " << door << " people inside after Method 2.\n";

    #if LOGGING
        trace::trace_end();
    #endif

    return 1;
}
