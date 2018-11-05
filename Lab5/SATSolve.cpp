#include <iostream>
#include <locale>
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

//Small class to allow comma separated numbers (i.e 1234 will display as 1,234)
class comma_numpunct : public std::numpunct<char>
{
    protected:
        char do_thousands_sep()   const {return ','; }
        std::string do_grouping() const {return "\3";}
};

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
    //Initialize the SAT Problem
    SAT_Problem sat(textFile);
    //Set cout to use commas
    std::locale comma_locale(std::locale(), new comma_numpunct());
    std::cout.imbue(comma_locale);
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
        std::cout << "\rAfter " << sat.GetBacktracks() << " backtracks, the problem was found to be unsatisfiable." << std::endl;
    }

    log.join();

    // End process
    return 0;
}
