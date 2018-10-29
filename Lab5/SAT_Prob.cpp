#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include "SAT_Prob.h"

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
    #pragma omp parallel for
    for (unsigned int i=0; i<clauses.size(); i++)
    {
        std::vector<bool_SAT> clauseOutput;
        auto sign = [](int x) {return x/abs(x);};

        //NOT gate    = -1*data
        //BUFFER gate =  1*data
        for( auto var:clauses[i] )
            clauseOutput.push_back( static_cast<bool_SAT>( sign(var) * vars[abs(var)-1] ) );

        //OR gate = max(data)
        clauseResults[i] = *std::max_element(clauseOutput.begin(), clauseOutput.end());
    }

    //AND gate = min(data)
    return *std::min_element(clauseResults.begin(), clauseResults.end());
}

/*
Solve()

Attempt to solve the current clauses, using a backtrack search algorithm. Returns
true if a solution is found, otherwise return false.
*/
bool SAT_Problem::Solve()
{
    //Ensure variables are all unset
    backtracks=0;
    for( auto &v:vars )
        v=Unset;
    //Backtrack search
    int i=0;
    while ( i <= int( vars.size() ) )
    {
        //this->Print();
        switch( this->Check() )
        {
            //We have a solution
            case True:
                return true;
            //Keep advancing forward
            case Unset:
                vars[i] = True;
                i++;
                break;
            //Backtrack to first "True" Value
            case False:
                i--;
                while( i >= 0)
                {
                    if( vars[i] == True )
                    {
                        vars[i] = False;
                        i++;
                        break;
                    }
                    else
                    {
                        vars[i] = Unset;
                        i--;
                        backtracks++;
                    }
                }
                if (i<0) return false; //No solution exists (backtracked past front)
                break;
        }
    }
    //Should exit in switch case; if not, problem is ill-conditioned
    return false;
}
