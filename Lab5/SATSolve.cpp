#include <iostream>
#include "SAT_Prob.h"

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
    if( sat.Solve() )
    {
        std::cout << "A solution was found. Note that the left-most number is variable 1, and X's are don't-cares." << std::endl << std::endl;
        sat.Print();
    }
    else
    {
        std::cout << "No solution could be found." << std::endl;
    }

    // End process
    return 0;
}
