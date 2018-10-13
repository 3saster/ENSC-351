#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <csignal>
#include <vector>
#include <utility>
#include <algorithm>

#include "tracelib.h"
#define LOGGING 1 //Set to one to turn on tracing

typedef std::string data;
typedef std::pair<data,int> kvPair;

std::vector<data> input_reader(std::ifstream& textFile)
{
    trace::trace_event_start("Input Reader","MapReduce");

    std::string word;
    std::vector<std::string> wordVector;
    while( textFile >> word )
    {
        wordVector.push_back(word);
    }

    trace::trace_event_end();

    return wordVector;
}

kvPair map(data word)
{
    trace::trace_event_start("Map","MapReduce");

    auto value = kvPair (word,1);

    trace::trace_event_end();
    return value;
}

kvPair reduce(std::vector<kvPair> groupVector)
{
    trace::trace_event_start("Reduce","MapReduce");

    kvPair reducedKV (groupVector.front().first,0);
    for(auto& kv : groupVector)
        reducedKV.second += kv.second;

    trace::trace_event_end();
    return reducedKV;
}

template<typename Map>
void output(const Map& m)
{
    trace::trace_event_start("Output","MapReduce");

    for(auto& p: m)
        std::cout << p.first << " : " << p.second << std::endl;

    trace::trace_event_end();
}

int main(int argc, char *argv[])
{
    /*    START LOGGING    */
    #if LOGGING
        trace::trace_start("wordCount_2.json");
        auto traceEnd = [] (int i) { trace::trace_instant_global("Program Aborted"); trace::trace_end(); exit(128+i); };
        std::signal(SIGINT, traceEnd); //^C
        std::signal(SIGABRT, traceEnd); //abort()
        std::atexit([](){ trace::trace_end(); }); //return
    #endif

    trace::trace_event_start("Word Count 2","Word Count 2");
    /*    OPEN FILE    */
    // Check for argument
    if(argc < 2)
    {
        std::cerr << "Error: Filename not given.\n";
        return 1;
    }
    // Open the actual file
    trace::trace_event_start("Open File","Word Count 2");
    std::ifstream textFile;
    textFile.open(argv[1]);
    if(!textFile.is_open())
    {
        std::cerr << "Error: Unable to open file \"" << argv[1] << "\".\n";
        return 1;
    }
    trace::trace_event_end();

    /*    GENERIC STUFF    */
    trace::trace_event_start("Count Words","Word Count 2");

    std::vector<data> dataVector;
    std::vector<kvPair> pairVector;
    std::vector<kvPair> reducedVector;

    dataVector = input_reader(textFile);
    for(auto& element : dataVector)
    {
        pairVector.push_back( map(element) );
    }
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
            kvPair pushPair = reduce( shortVector );
            reducedVector.push_back(pushPair);
            groupFront = groupBack;
        }
    }
    output(reducedVector);

    trace::trace_event_end();

    // End process
    trace::trace_event_end();
    return 0;
}
