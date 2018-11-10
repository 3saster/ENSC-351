# Declaration of variables
CC = g++
CC_FLAGS = -std=c++11 -pthread -O3
CC_EXTRA =

main: SATSolve.cpp SAT_Prob.cpp
	$(CC) $(CC_FLAGS) $(CC_EXTRA) SATSolve.cpp SAT_Prob.cpp -o SATSolve

# Clean
clean:
	rm -f SATSolve
