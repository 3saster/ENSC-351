#include <iostream>
#include <fstream>
#include <map>
#include <string>

template<typename Map>
void print_map(const Map& m)
{
   for(auto& p: m)
       std::cout << p.first << " : " << p.second << std::endl;
}

int main(int argc, char *argv[])
{
    // Check for argument
    if(argc < 2)
    {
        std::cerr << "Error: Filename not given.\n";
        return 1;
    }

    // Open file
    std::ifstream traceFile;
    traceFile.open(argv[1]);
    if(!traceFile.is_open())
    {
        std::cerr << "Error: Unable to open file \"" << argv[1] << "\".\n";
        return 1;
    }

    // Count Words
    std::string word;
    std::map<std::string, int> wordCount;
    while( traceFile >> word )
    {
        wordCount[word]++;
    }

    // Print Map
    print_map(wordCount);
    return 0;
}
