#ifndef SAT_PROB_H
#define SAT_PROB_H

#include <vector>
#include <algorithm>
#include <fstream>
#include <atomic>

//Three-valued logic under Kleene Logic
typedef enum { False=-1, Unset=0, True=1 } bool_SAT;
namespace tribool
{
    inline bool_SAT NOT(bool_SAT b)                     { return static_cast<bool_SAT> ( -1*b ); }
    inline bool_SAT AND(const std::vector<bool_SAT> &b) { return *std::min_element(b.begin(), b.end()); }
    inline bool_SAT OR (const std::vector<bool_SAT> &b) { return *std::max_element(b.begin(), b.end()); }
}

class SAT_Problem
{
    private:
        std::vector<bool_SAT> vars;
        std::vector< std::vector<int> > clauses;
        std::atomic<long long> backtracks {0};
        bool solved = false;
        bool threadSolve(int tid, int threadDepth);

    public:
        SAT_Problem (std::ifstream& textFile);
        SAT_Problem(const SAT_Problem& sat);
        bool_SAT Check();
        bool Solve();
        void Print();
        long long int GetBacktracks() {return backtracks;}
};

#endif //SAT_PROB_H
