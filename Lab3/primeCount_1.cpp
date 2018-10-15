#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cstdlib>
#include <csignal>
#include <cmath>

#include "tracelib.h"
#define LOGGING 1 //Set to one to turn on tracing

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

template<typename Map>
void print_map(const Map& m)
{
   for(auto& p: m)
       std::cout << p.first << " : " << p.second << std::endl;
}

int main(int argc, char *argv[])
{
    /*    START LOGGING    */
    #if LOGGING
        trace::trace_start("primeCount_1.json");
        auto traceEnd = [] (int i) { trace::trace_instant_global("Program Aborted"); trace::trace_end(); exit(128+i); };
        std::signal(SIGINT, traceEnd); //^C
        std::signal(SIGABRT, traceEnd); //abort()
        std::atexit([](){ trace::trace_end(); }); //return
    #endif

    trace::trace_event_start("Prime Count 1","Prime Count 1");
    // Check for argument
    if(argc < 2)
    {
        std::cerr << "Error: Filename not given.\n";
        return 1;
    }

    // Open file
    trace::trace_event_start("Open File","Prime Count 1");
    std::ifstream textFile;
    textFile.open(argv[1]);
    if(!textFile.is_open())
    {
        std::cerr << "Error: Unable to open file \"" << argv[1] << "\".\n";
        return 1;
    }
    trace::trace_event_end();

    // Count Words
    trace::trace_event_start("Count Primes","Prime Count 1");
    long long int num;
    std::map<std::string, int> primeCount;
    while( textFile >> num )
    {
        if( is_prime(num) )
            primeCount["Prime Numbers"]++;
        else
            primeCount["Composite Numbers"]++;
    }
    trace::trace_event_end();

    // Print Map
    trace::trace_event_start("Print Prime Count","Prime Count 1");
    print_map(primeCount);
    trace::trace_event_end();

    trace::trace_event_end();
    return 0;
}
