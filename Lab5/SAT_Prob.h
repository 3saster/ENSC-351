#ifndef SAT_PROB_H
#define SAT_PROB_H

#include <vector>
#include <fstream>

//Three-valued logic under Kleene Logic
typedef enum { False=-1, Unset=0, True=1 } bool_SAT;

class SAT_Problem
{
    private:
        std::vector<bool_SAT> vars;
        std::vector< std::vector<int> > clauses;
        long long int backtracks = 0;

    public:
        SAT_Problem (std::ifstream& textFile);
        bool_SAT Check();
        bool Solve();
        void Print();
        long long int GetBacktracks() {return backtracks;}
};

#endif //SAT_PROB_H
