#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <thread>
#include "SAT_Prob.h"

using namespace tribool;

/*
Constructor

Takes in an ifstream, and fills the vectors based on the clauses and numbers
given. Currently assumes no comments.
*/
SAT_Problem::SAT_Problem (std::ifstream& textFile)
{
    int word;
    //For now, assume file is valid CNF FILE
    std::string filler;
    textFile >> filler; //Ignore first word
    textFile >> filler; //Ignore second word

    //Initialize vars to have the appropriate number of variables unset
    textFile >> word;
    vars.assign(word, Unset);

    textFile >> word;
    int clause=1;
    while( clause <= word )
    {
        int condition;
        std::vector<int> conditionVector;
        while( textFile >> condition && condition != 0 )
            conditionVector.push_back(condition);
        clauses.push_back(conditionVector);
        clause++;
    }
}

SAT_Problem::SAT_Problem(const SAT_Problem& sat)
{
    vars = sat.vars;
    clauses = sat.clauses;
    backtracks = sat.backtracks.load();
    solved = sat.solved;
}

/*
Print()

Print the currect value of the vars vectors. Prints True as 1, False as 0, and
X as unset, with the leftmost value being vars[0].
*/
void SAT_Problem::Print()
{
    for( auto p:vars )
    {
        switch(p)
        {
            case True:
                std::cout << "1";
                break;
            case Unset:
                std::cout << "X";
                break;
            case False:
                std::cout << "0";
                break;
        }
    }
    std::cout << std::endl;
}

/*
Check()

Check what the current vars return for the current clauses, and return this value.
Note that we are using a three-valued boolean under Kleene logic.
*/
bool_SAT SAT_Problem::Check()
{
    std::vector<bool_SAT> clauseResults ( clauses.size(), Unset );

    for (unsigned int i=0; i<clauses.size(); i++)
    {
        std::vector<bool_SAT> clauseOutput;
        bool ORskip = false;

        for( auto var:clauses[i] )
        {
            auto value = vars[abs(var)-1];
            value = var > 0 ? value : NOT(value);
            //If value is True, this clause will be True, so leave early
            if(value == True)
            {
                clauseResults[i] = True;
                ORskip = true;
                break;
            }
            clauseOutput.push_back(value);
        }

        if(!ORskip)
        {
            auto clauseValue = OR(clauseOutput);
            //If value is false, the final AND will return False, so leave early
            if(clauseValue == False)
                return False;
            else
                clauseResults[i] = clauseValue;
        }
    }

    return AND(clauseResults);
}

/*
Solve()

Attempt to solve the current clauses, using a backtrack search algorithm. Returns
true if a solution is found, otherwise return false.
*/
bool SAT_Problem::Solve()
{
    //Get depth of parallel binary tree search (min of either round down number of cores, or number of variables )
    int threadDepth = std::min( log2(std::thread::hardware_concurrency()), double( vars.size() ) );
    int threadNum = pow(2,threadDepth);

    //Ensure variables are all unset
    backtracks=0;
    solved=false;
    for( auto &v:vars )
        v=Unset;
    //Backtrack search
    std::thread *threads = new std::thread[threadNum];
    for(int tid=0; tid<threadNum; tid++)
    {
        threads[tid] = std::thread(&SAT_Problem::threadSolve,this, tid,threadDepth);
    }

    for(int tid=0; tid<threadNum; tid++)
        threads[tid].join();

    delete [] threads;

    if(solved)
        return true;
    else
        return false;
}

bool SAT_Problem::threadSolve(int tid, int threadDepth)
{
    SAT_Problem copySAT = *this;
    auto *copyVars = &copySAT.vars;
    //Lock starting values
    for(int i=0;i<threadDepth;i++)
        (*copyVars)[i] = (tid>>i)%2 ? True : False;

    //Backtrack search
    int i=threadDepth;
    while ( !solved && i <= int( (*copyVars).size() ) )
    {
        //this->Print();
        switch( copySAT.Check() )
        {
            //We have a solution
            case True:
                solved=true;
                vars = *copyVars;
                return true;
            //Keep advancing forward
            case Unset:
                (*copyVars)[i] = True;
                i++;
                break;
            //Backtrack to first "True" Value
            case False:
                i--;
                while( i >= threadDepth)
                {
                    if( (*copyVars)[i] == True )
                    {
                        (*copyVars)[i] = False;
                        i++;
                        break;
                    }
                    else
                    {
                        (*copyVars)[i] = Unset;
                        i--;
                        backtracks++;
                    }
                }
                if (i<threadDepth) return false; //No solution exists (backtracked past front)
                break;
        }
        if(solved) return true;
    }
    //Should exit in switch case; if not, something went wrong
    std::cerr << "Error in SAT_Problem::Solve(): Exited in an unexpected way." << std::endl;
    return false;
}
