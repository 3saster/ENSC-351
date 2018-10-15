# Declaration of variables
CC = g++
CC_FLAGS = -std=c++11 -pthread
CC_EXTRA =

main:
	@echo 'Call "make wordcount" for word counter or "make primecount" for prime counter.'
# make the wordcount set
wordcount: wordCount_1.cpp wordCount_2.cpp
	$(CC) $(CC_FLAGS) $(CC_EXTRA) wordCount_1.cpp -o wordCount_1
	$(CC) $(CC_FLAGS) $(CC_EXTRA) wordCount_2.cpp -o wordCount_2

# make the primecount set
primecount:
	$(CC) $(CC_FLAGS) $(CC_EXTRA) primeCount_1.cpp -o primeCount_1
	$(CC) $(CC_FLAGS) $(CC_EXTRA) primeCount_2.cpp -o primeCount_2
	./makePrime.sh

# Clean
clean:
	rm -f wordCount_1 wordCount_2 primeCount_1 primeCount_2 *.json prime.txt
