#ifndef SAT_PROB_H
#define SAT_PROB_H

#include <vector>
#include <fstream>

typedef enum { False=-1, Unset=0, True=1 } bool_SAT;

class SAT_Problem
{
    private:
        std::vector<bool_SAT> vars;
        std::vector< std::vector<int> > clauses;

    public:
        SAT_Problem (std::ifstream& textFile);
        bool_SAT Check();
        bool Solve();
        void Print();
};

#endif //SAT_PROB_H
