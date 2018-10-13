#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <cstdlib>
#include <csignal>

#include "tracelib.h"
#define LOGGING 1 //Set to one to turn on tracing

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
        trace::trace_start("wordCount_1.json");
        auto traceEnd = [] (int i) { trace::trace_instant_global("Program Aborted"); trace::trace_end(); exit(128+i); };
        std::signal(SIGINT, traceEnd); //^C
        std::signal(SIGABRT, traceEnd); //abort()
        std::atexit([](){ trace::trace_end(); }); //return
    #endif

    trace::trace_event_start("Word Count 1","Word Count 1");
    // Check for argument
    if(argc < 2)
    {
        std::cerr << "Error: Filename not given.\n";
        return 1;
    }

    // Open file
    trace::trace_event_start("Open File","Word Count 1");
    std::ifstream textFile;
    textFile.open(argv[1]);
    if(!textFile.is_open())
    {
        std::cerr << "Error: Unable to open file \"" << argv[1] << "\".\n";
        return 1;
    }
    trace::trace_event_end();

    // Count Words
    trace::trace_event_start("Count Words","Word Count 1");
    std::string word;
    std::map<std::string, int> wordCount;
    while( textFile >> word )
    {
        wordCount[word]++;
    }
    trace::trace_event_end();

    // Print Map
    trace::trace_event_start("Print Word Count","Word Count 1");
    print_map(wordCount);
    trace::trace_event_end();

    trace::trace_event_end();
    return 0;
}
