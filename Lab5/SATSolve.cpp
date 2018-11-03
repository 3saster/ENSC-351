#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include "SAT_Prob.h"

#ifndef LOGTIMER
    #define LOGTIMER 2
#endif

#define DIV 200

std::atomic<bool> done (false);

void logger(SAT_Problem &sat)
{
    int i;
    float step = 1000 * LOGTIMER / DIV;
    //done must be checked many times due to the nature of threading
    while(!done)
    {
        i=0;
        //Trick to not sleep the full LOGTIMER amount if the SAT is solved early
        while( i < DIV && !done)
        {
            std::this_thread::sleep_for( std::chrono::milliseconds( int(step) ) );
            i++;
        }
        if(!done)
        {
            std::cout << "\r" << sat.GetBacktracks() << " backtracks done so far." << std::flush;
        }
    }
}

int main(int argc, char *argv[])
{
    /*    OPEN FILE    */
    // Check for argument
    if(argc < 2)
    {
        std::cerr << "Error: Filename not given.\n";
        return 1;
    }
    // Open the actual file
    std::ifstream textFile;
    textFile.open(argv[1]);
    if(!textFile.is_open())
    {
        std::cerr << "Error: Unable to open file \"" << argv[1] << "\".\n";
        return 1;
    }

    /*    SAT Problem Solver    */
    SAT_Problem sat(textFile);
    //Start Logger
    std::thread log(logger,std::ref(sat));

    //SAT Solver
    if( sat.Solve() )
    {
        done = true;
        std::cout << "\rA solution was found after " << sat.GetBacktracks() << " backtracks." << std::endl
                  << "Note that the left-most number is variable 1, and X's are don't-cares." << std::endl << std::endl;
        sat.Print();
    }
    else
    {
        done = true;
        std::cout << "\rNo solution could be found." << std::endl;
    }

    log.join();

    // End process
    return 0;
}
