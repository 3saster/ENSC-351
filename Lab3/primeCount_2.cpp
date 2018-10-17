#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <csignal>
#include <vector>
#include <utility>
#include <algorithm>
#include <mutex>

#include "threadpool/ctpl_stl.h"
#include "tracelib.h"
#define LOGGING 1 //Set to one to turn on tracing
#ifndef NUM_THREADS
    #define NUM_THREADS std::thread::hardware_concurrency()*3
#endif

typedef long long int data;
typedef std::pair<std::string,int> kvPair;
std::mutex mtx;

bool is_prime(long long int n,int tid=1)
{
    trace::trace_event_start("Check Prime","Check Prime",tid);
    bool retValue = true;

    if (n < 2)
        retValue = false;

    if ( n%2 == 0 )
        retValue = false;

    if(retValue)
    {
        long long int mid = sqrt(n);
        for(long long int i = 3; i <= mid; i=i+2)
        {
            if( n%i == 0 )
                retValue = false;
        }
    }
    trace::trace_event_end(tid);
    return retValue;
}

/*    MapReduce Components    */
std::vector<data> input_reader(std::ifstream& textFile, int tid=1)
{
    trace::trace_event_start("Input Reader","MapReduce",tid);

    data word;
    std::vector<data> wordVector;
    while( textFile >> word )
    {
        wordVector.push_back(word);
    }

    trace::trace_event_end(tid);

    return wordVector;
}

kvPair map(data word, int tid=1)
{
    trace::trace_event_start("Map","MapReduce",tid);
    kvPair value;

    if ( is_prime(word,tid) )
        value = kvPair ("Prime Numbers",1);
    else
        value = kvPair ("Composite Numbers",1);

    trace::trace_event_end(tid);
    return value;
}

kvPair reduce(std::vector<kvPair> groupVector, int tid=1)
{
    trace::trace_event_start("Reduce","MapReduce",tid);

    kvPair reducedKV (groupVector.front().first,0);
    for(auto& kv : groupVector)
        reducedKV.second += kv.second;

    trace::trace_event_end(tid);
    return reducedKV;
}

template<typename Map>
void output(const Map& m, int tid=1)
{
    trace::trace_event_start("Output","MapReduce",tid);

    for(auto& p: m)
        std::cout << p.first << " : " << p.second << std::endl;

    trace::trace_event_end(tid);
}

/*    Threading Stuff    */
void thread_map(int tid, data element, std::vector<kvPair>& pairVector)
{
    auto mappedElement = map(element, tid+2);
    mtx.lock();
    pairVector.push_back( mappedElement );
    mtx.unlock();
}

void thread_reduce(int tid, std::vector<kvPair> groupVector, std::vector<kvPair>& reducedVector)
{
    auto pushPair = reduce(groupVector, tid+2);
    mtx.lock();
    reducedVector.push_back( pushPair );
    mtx.unlock();
}

void mapReduce(std::ifstream& textFile)
{
    trace::trace_event_start("Count Primes","Prime Count 2");
    std::vector<data> dataVector;
    std::vector<kvPair> pairVector;
    std::vector<kvPair> reducedVector;
    ctpl::thread_pool tp(NUM_THREADS);

    dataVector = input_reader(textFile);
    for(auto& element : dataVector)
    {
        if(NUM_THREADS > 0)
            tp.push(thread_map, element,std::ref(pairVector));
        else
            thread_map(-1,element,pairVector);
    }
    while(tp.is_busy()){} //Spinlock until threads are done mapping

    std::sort (pairVector.begin(), pairVector.end());
    auto groupFront = pairVector.begin();
    auto groupBack  = groupFront;
    while (groupBack <= pairVector.end())
    {
        if(groupFront->first == groupBack->first) //If data is the same
            groupBack++;
        else
        {
            std::vector<kvPair> shortVector (groupFront,groupBack);
            if(NUM_THREADS > 0)
                tp.push(thread_reduce, shortVector,std::ref(reducedVector));
            else
                thread_reduce(-1,shortVector,reducedVector);
            groupFront = groupBack;
        }
    }

    while(tp.is_busy()){} //Spinlock until threads are done reducing
    trace::trace_event_end();

    std::sort(reducedVector.begin(), reducedVector.end());
    output(reducedVector);
}

int main(int argc, char *argv[])
{
    /*    START LOGGING    */
    #if LOGGING
        trace::trace_start("primeCount_2.json");
        auto traceEnd = [] (int i) { trace::trace_instant_global("Program Aborted"); trace::trace_end(); exit(128+i); };
        std::signal(SIGINT, traceEnd); //^C
        std::signal(SIGABRT, traceEnd); //abort()
        std::atexit([](){ trace::trace_end(); }); //return
    #endif

    trace::trace_event_start("Prime Count 2","Prime Count 2");
    /*    OPEN FILE    */
    // Check for argument
    if(argc < 2)
    {
        std::cerr << "Error: Filename not given.\n";
        return 1;
    }
    // Open the actual file
    trace::trace_event_start("Open File","Prime Count 2");
    std::ifstream textFile;
    textFile.open(argv[1]);
    if(!textFile.is_open())
    {
        std::cerr << "Error: Unable to open file \"" << argv[1] << "\".\n";
        return 1;
    }
    trace::trace_event_end();

    /*    MapReduce    */
    mapReduce(textFile);

    // End process
    trace::trace_event_end();
    return 0;
}
