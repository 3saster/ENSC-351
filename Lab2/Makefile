# Declaration of variables
CC = g++
CC_FLAGS = -std=c++11 -pthread
NUM_THREADS = 1000

# Make
main: main.cpp
	$(CC) $(CC_FLAGS) main.cpp -DNUM_THREADS=$(NUM_THREADS) -o Lab2
 
# Clean
clean:
	rm -f Lab2 trace.json short_trace.json
	
# Shorten the trace.json to show only the main thread
short:
	echo [ > short_trace.json; awk '/"tid": 1,/' trace.json >> short_trace.json; echo ] >> short_trace.json;