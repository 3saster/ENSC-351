# Declaration of variables
CC = g++
CC_FLAGS = -std=c++11 -pthread

# Make
main: wordCount_1.cpp wordCount_2.cpp
	$(CC) $(CC_FLAGS) wordCount_1.cpp -o wordCount_1
	$(CC) $(CC_FLAGS) wordCount_2.cpp -o wordCount_2

# Clean
clean:
	rm -f wordCount_1 wordCount_2 *.json
