#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

typedef std::string data;
typedef std::pair<data,int> kvPair;

/*    MapReduce Components    */
std::vector<data> input_reader(std::ifstream& textFile)
{
    data word;
    std::vector<data> wordVector;
    while( textFile >> word )
    {
        wordVector.push_back(word);
    }
    return wordVector;
}

kvPair map(data word)
{
    return kvPair (word,1);
}

kvPair reduce(std::vector<kvPair> groupVector)
{
    kvPair reducedKV (groupVector.front().first,0);
    for(auto& kv : groupVector)
        reducedKV.second += kv.second;
    return reducedKV;
}

template<typename Map>
void output(const Map& m)
{
    for(auto& p: m)
        std::cout << p.first << " : " << p.second << std::endl;
}

/*    MapReduce Framework    */
void mapReduce(std::ifstream& textFile)
{
    std::vector<data> dataVector;
    std::vector<kvPair> pairVector;
    std::vector<kvPair> reducedVector;
    std::vector<std::vector<kvPair>> clumpVector;

    //Input Reader
    dataVector = input_reader(textFile);

    //Map
    #pragma omp parallel for
    for(auto element = dataVector.begin(); element < dataVector.end(); element++)
    {
    auto mappedElement = map(*element);
    #pragma omp critical
        pairVector.push_back( mappedElement );
    }

    //Get Clumps
    std::sort(pairVector.begin(), pairVector.end());
    auto groupFront = pairVector.begin();
    auto groupBack  = groupFront;
    while (groupBack <= pairVector.end())
    {
        if(groupFront->first == groupBack->first) //If data is the same
            groupBack++;
        else
        {
            std::vector<kvPair> shortVector (groupFront,groupBack);
            clumpVector.push_back(shortVector);
            groupFront = groupBack;
        }
    }

    //Reduce
    int i=0;
    #pragma omp parallel for
    for(auto clump = clumpVector.begin(); clump < clumpVector.end(); clump++)
    {
    auto pushPair = reduce(*clump);
    #pragma omp critical
        reducedVector.push_back( pushPair );
    }
    
    //Output
    std::sort(reducedVector.begin(), reducedVector.end());
    output(reducedVector);
}

/*    MAIN    */
int main(int argc, char *argv[])
{
    /*    OPEN FILE    */
    // Check for argument
    if(argc < 2)
    {
        std::cerr << "Error: Filename not given.\n";
        return 1;
    }
    // Open the actual file
    std::ifstream textFile;
    textFile.open(argv[1]);
    if(!textFile.is_open())
    {
        std::cerr << "Error: Unable to open file \"" << argv[1] << "\".\n";
        return 1;
    }

    /*    MapReduce    */
    mapReduce(textFile);

    // End process
    return 0;
}
