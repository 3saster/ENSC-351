#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <csignal>
#include <functional>

#include "tracelib.h"
#define LOGGING 1 //Set to one to turn on tracing

/*  CONSTANTS   */
#ifndef NUM_THREADS
    #define NUM_THREADS 1000
#endif

/*  GLOBALS */
int door(0); //intentionally non-atomic
std::atomic<int> method_init(0);
//Used in method 2
bool cellArray[NUM_THREADS+1] = { false };
//Used in method 3
int tickets[NUM_THREADS+1] = { 0 }; //Ensure going beyond the end hits a non-existent thread (0)
std::atomic<int*> nowServing(&tickets[0]);
std::atomic<int> lineSpot(0);

/*  MUTEXES & CONDITION VARIABLES   */
//Initialisation
std::mutex m_init;
std::condition_variable cv_init;
//Main Thread Waiting
std::mutex m_wait;
std::condition_variable cv_wait;
//Method 1
std::mutex m_1;
//Method 2
std::mutex m_2;
std::condition_variable cv_2;
//Method 3
std::mutex m_3_write;
std::mutex m_3;
std::condition_variable cv_3;

/*  HELPER FUNCTIONS    */
void wait_for_release(const int i)
{
    std::unique_lock<std::mutex> lck(m_init);
    while( !(method_init == i) )
        cv_init.wait(lck);
}

void release_threads(const int i)
{
    std::unique_lock<std::mutex> lck(m_init);
    method_init = i;
    cv_init.notify_all();
}

void wait_for_method_finish()
{
    std::unique_lock<std::mutex> lck(m_wait);
    while( !(door == NUM_THREADS) )
        cv_wait.wait(lck);
}

void wait_your_turn( std::function<bool()> condition, std::mutex &m, std::condition_variable &cv)
{
    std::unique_lock<std::mutex> lck(m);
    while( !condition() )
        cv.wait(lck); 
}

/*  METHOD FUNCTIONS   */
//Method 1: A simple lock (in theory unnecessary if door was atomic)
void method_1(const unsigned int tid)
{
    wait_for_release(1);

    m_1.lock();
    trace::trace_event_start("Method 1 Increment", "Method 1", tid);
    door++;
    trace::trace_event_end(tid);
    cv_wait.notify_one();
    m_1.unlock();
}

//Method 2: Essentially, thread 2 increments door, then thread 3, and so forth
void method_2(const unsigned int tid)
{
    wait_for_release(2);

    //Match tid to cell array; i.e. thread 2 = 0 in array
    int true_tid = tid-2;
    wait_your_turn( [&]() {return cellArray[true_tid];}, m_2, cv_2 );

    trace::trace_event_start("Method 2 Increment", "Method 2", tid);
    door++;
    trace::trace_event_end(tid);
    
    //Allow next person to enter
    {
        std::unique_lock<std::mutex> lck(m_2);
        cellArray[true_tid+1] = true;
        cv_2.notify_all();
        cv_wait.notify_one();
    }
}


//Method 3: Ticket Lock Implementation
void method_3(const unsigned int tid)
{
    wait_for_release(3);
    
    //Add ticket to ticket list
    m_3_write.lock();
    tickets[lineSpot++] = tid;
    m_3_write.unlock();

    wait_your_turn( [&]() {return *nowServing == tid;}, m_3, cv_3 );

    trace::trace_event_start("Method 3 Increment", "Method 3", tid);
    door++;
    trace::trace_event_end(tid);
    
    //Allow next person to enter
    {
        std::unique_lock<std::mutex> lck(m_3);
        nowServing++;
        cv_3.notify_all();
        cv_wait.notify_one();
    }
}

//Combination of methods
void lab2_methods(const unsigned int tid, const int method)
{
    switch(method)
    {
        case 1: method_1(tid); break;
        case 2: method_2(tid); break;
        case 3: method_3(tid); break;
    }
}

/*  MAIN THREAD  */
int main(int argc, char *argv[])
{
    /*  DETERMINE METHOD    */
    int method;
    
    if( argc != 2 ) //Default to Method 1 if no argument is provided
    {
        std::cerr << "Error: An argument must be provided (either 1, 2, or 3).\n";
        exit(1);
    }
    else
    {
        try{ method = std::stoi(argv[1]); }
        catch (const std::exception& e)
        {
            std::cerr << "Error: The argument could not be interpreted (should be 1, 2, or 3).\n";
            exit(2);
        }
        if ( method != 1 && method != 2 && method != 3 )
        {
            std::cerr << "Error: The argument is not a valid method (valid methods are 1, 2, or 3).\n";
            exit(3);
        }    
    }
    
    /*  START LOGGING   */
    #if LOGGING
        std::string name = "method_"+std::to_string(method)+".json";
        trace::trace_start(name.c_str());
        auto traceEnd = [] (int i) { trace::trace_end(); exit(128+i); };
        std::signal(SIGINT, traceEnd); //^C
        std::signal(SIGABRT, traceEnd); //abort()
    #endif
    
    /*  SETUP   */
    //Create Array of Threads
    std::thread threadArray[NUM_THREADS];
    
    trace::trace_event_start("Thread Initialization", "Init");
    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i] = std::thread(lab2_methods,i+2, method); //First thread created should be thread 2
    }
    trace::trace_event_end();
    
    std::cout << NUM_THREADS << " threads have been initialized.\n";
    
    /*  METHOD 1    */
    if( method == 1 )
    {  
        trace::trace_event_start("Method 1", "Method 1");
        release_threads(1);
        wait_for_method_finish();
        trace::trace_event_end();

        std::cout << "There are " << door << " people inside after Method 1.\n";
    }
    
    /*  METHOD 2    */
    if( method == 2 )
    {
        trace::trace_event_start("Method 2", "Method 2");
        release_threads(2);
        //Let first guy in
        {
            std::unique_lock<std::mutex> lck(m_2);
            cellArray[0] = true;
            cv_2.notify_all();
        }
        wait_for_method_finish();
        trace::trace_event_end();

        std::cout << "There are " << door << " people inside after Method 2.\n";
    }
    
    /*  METHOD 3    */
    if( method == 3 )
    {
        trace::trace_event_start("Method 3", "Method 3");
        release_threads(3);
        wait_for_method_finish();
        trace::trace_event_end();

        std::cout << "There are " << door << " people inside after Method 3.\n";
    }
    
    /*  JOIN THREADS    */
    trace::trace_event_start("Join Threads", "Finish");
    for(int i=0; i<NUM_THREADS; i++)
    {
        threadArray[i].join();
    }
    trace::trace_event_end();

    /*  END LOGGING */
    #if LOGGING
        trace::trace_end();
    #endif

    return 0;
}
