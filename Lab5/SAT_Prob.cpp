#include <iostream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <thread>
#include <mutex>
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

    //Read each clause into clauses member
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

/*
Copy Constructor

Needed since atomic does not have a copy constructor.
*/
SAT_Problem::SAT_Problem(const SAT_Problem& sat)
{
    vars = sat.vars;
    clauses = sat.clauses;
    backtracks = sat.backtracks.load();
    solved = sat.solved;
}

/*
Print()

Print the currect value of the vars vectors, in the form 1 -2 3...
Note that Unset will be shown as true (this is meant to print a solution).
*/
void SAT_Problem::Print()
{
    for( unsigned int i=0; i<vars.size(); i++ )
    {
        switch(vars[i])
        {
            case True:
            case Unset:
                std::cout << " " << i+1;
                break;
            case False:
                std::cout << " -" << i+1;
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

    //For each clause...
    for (unsigned int i=0; i<clauses.size(); i++)
    {
        std::vector<bool_SAT> clauseOutput;
        bool ORskip = false;

        //For each variable in this clause...
        for( auto var:clauses[i] )
        {
            //...get the value of that part of the clause (negating if necessary)
            // and push it into the output vector
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

        //OR the results of each variable in the clause
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

    //AND the results of the clauses
    return AND(clauseResults);
}

/*
Solve()

Attempt to solve the current clauses, using a backtrack search algorithm. Returns
true if a solution is found, otherwise return false.
*/
bool SAT_Problem::Solve()
{
    //Get depth of parallel binary tree search (min of either round down number of cores, or number of variables)
    //i.e. if we have 4-7 cores, we can do 2 levels deep (4 branches)
    int threadDepth = std::min( log2(std::thread::hardware_concurrency()), double( vars.size() ) );
    int threadNum = pow(2,threadDepth);

    //Ensure variables are all unset
    backtracks=0;
    solved=false;
    for( auto &v:vars )
        v=Unset;

    //Backtrack search passed to group of threads
    std::thread *threads = new std::thread[threadNum];
    for(int tid=0; tid<threadNum; tid++)
        threads[tid] = std::thread(&SAT_Problem::threadSolve,this, tid,threadDepth);

    //Wait for threads to rejoin
    for(int tid=0; tid<threadNum; tid++)
        threads[tid].join();
    delete [] threads;

    //Return true or false depending on if a solution was found
    if(solved)
        return true;
    else
        return false;
}

/*
threadSolve()

This performs a backtrack search on a copy of the SAT_Problem, where certain variables
are locked.

For example, given a thread depth of 3 (i.e 8 cores), thread 0 does a backtrack search
where the first three values are locked to 000, thread 1 does 100, thread 2 does 010, etc.
*/
static std::mutex m;
bool SAT_Problem::threadSolve(int tid, int threadDepth)
{
    //Create copy of SAT problem and its vars
    SAT_Problem copySAT = *this;
    auto *copyVars = &copySAT.vars;

    //Lock starting values in
    for(int i=0;i<threadDepth;i++)
        (*copyVars)[i] = (tid>>i)%2 ? True : False;

    //Backtrack search
    int i=threadDepth;
    while ( !solved && i <= int( (*copyVars).size() ) )
    {
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
            //Backtrack to first (not locked) "True" Value
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
                if (i<threadDepth) return false; //No solution exists (backtracked past locked values)
                break;
        }
    }
    if(solved) return true;
    //Should exit in switch case or line above; if not, something went wrong
    m.lock();
    std::cerr << "Error in SAT_Problem::threadSolve() on thread " << tid << ": Exited in an unexpected way." << std::endl;
    m.unlock();
    return false;
}
