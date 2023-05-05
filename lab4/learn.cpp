/*
* Lab4: Hemant Ramawat(hr2378)
* 
* Use ReadMe file for how to compile and run
* 
*/

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <deque>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <float.h>
#include <limits.h>
#include <sstream>
#include <cmath>

using namespace std;

class Entry{
public:
    string label = "";
    vector<int> attributes;

    Entry(){}
};

enum ALGO{
    BAYES,
    KNN,
    KMEANS
};

enum MODE{
    TRAIN,
    TEST
};

bool DEBUG = true;
// string inputFile;
string inputFile = "tests/1_knn1.train.txt";
ALGO algo = ALGO::BAYES;
MODE mode;
bool verbose = false;
int attrLength = 0;
int k_knn = 0;
int c_laplace = 0;
bool useEuclid = true;
set<string> labels;
vector<Entry*> neighbors;
vector<Entry*> testEntries;
vector<string> predictions;
vector<string> groundTruth;

void PrintUsage(string error){
    printf("%s\n",error.c_str());
    printf("Usage:\n");
    printf("\t./learn -train <input-file.txt>\n\n");
    printf("Example:\n");
    printf("\t./learn -train some-input.txt 0,500 200,200 1000,1000\n");
}

// trim whitespace from start of string
static inline void ltrim(std::string &s){
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch){
        return !std::isspace(ch);
    }));
}

// trim whitespace from end of string
static inline void rtrim(std::string &s){
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch){
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends
static inline void trim(std::string &s){
    rtrim(s);
    ltrim(s);
}

bool SortByDistance(pair<double, string> p1, pair<double, string> p2){
    return p1.first < p2.first;
}

string KNNPredictEntryLable(Entry* testEntry){
    
    string predictedLabel = "";

    vector<pair<double, string>> distances;
    for(auto n = neighbors.begin(); n != neighbors.end(); n++){
        Entry* neighborEntry = *n;
        double currentDistance = 0;
        for (int i = 0; i < attrLength; i++){
            if(useEuclid){
                currentDistance += pow(testEntry->attributes[i] - neighborEntry->attributes[i], 2);
            } else {
                currentDistance += abs(testEntry->attributes[i] - neighborEntry->attributes[i]);
            }
        }
        distances.push_back(make_pair(currentDistance, neighborEntry->label));
    }

    sort(distances.begin(), distances.end(), SortByDistance);

    unordered_map<string,int> votes;
    int maxVotes = 0;
    for (auto l : labels){
        votes[l] = 0;
    }
    for (int i = 0; i < k_knn; i++){
        votes[distances[i].second]++;
    }
    for (auto l : labels){
        if(votes[l] > maxVotes){
            maxVotes = votes[l];
            predictedLabel = l;
        }
    }

    return predictedLabel;
}

void ParseKNNInput(const string inputFileName, bool isTestInput){
    ifstream fin;
    string line;
    if(DEBUG){ printf("parsing input file: %s\n", inputFileName.c_str());}

    try
    {
        fin.open(inputFileName);
    }
    catch(std::exception const& e)
    {
        printf("There was an error in opening input file %s: %s\n", inputFileName.c_str(), e.what());
        exit(1);
    }

    string token = "", lastToken = "";
    int entryCount = 0;
    
    while(getline(fin, line)){
        // Read a line from input file
        trim(line);
        if(DEBUG){ printf("Read line: '%s'\n", line.c_str());}
        if(line.empty() || line.at(0) == '#'){ continue;}

        entryCount++;

        stringstream s(line);
        Entry* e = new Entry();
        bool skippedFirstToken = false;
        int currentEntryLength = 0;

        while(getline(s, token, ',')){
            trim(token);
            // if(DEBUG){ printf("token '%s'\n",token.c_str());}
            currentEntryLength++;
            if(!skippedFirstToken){
                skippedFirstToken = true;
                lastToken = token;
                continue;
            }
            // if(DEBUG){ printf("last token '%s'\n",lastToken.c_str());}
            e->attributes.push_back(stoi(lastToken));
            lastToken = token;
        }
        
        if(!isTestInput){
            e->label = token;
            neighbors.push_back(e);
        } else {
            groundTruth.push_back(token);
            predictions.push_back(KNNPredictEntryLable(e));
        }
        if (attrLength == 0){
            attrLength = currentEntryLength;
        } else if (attrLength != currentEntryLength){
            printf("Error: inconsistent attribute length for entry of label %s at row %d\nexpected %d found %d", token.c_str(), entryCount, attrLength, currentEntryLength);
            exit(1);
        }
    }

    fin.close();
}

void DisplayKNNInput(){

    int rowcount = 0;
    printf("Input:\n");
    for (auto n : neighbors){
        rowcount++;
        printf("%d: %s->",rowcount, n->label.c_str());
        int i = 0;
        for (; i < n->attributes.size()-1; i++){
            printf("%d,",n->attributes[i]);
        }
        printf("%d\n", n->attributes[i]);
    }
}

void SolveKNN(){
    string testFile = "tests/1_knn1.test.txt";
    ParseKNNInput(testFile, true);
}

void PrintKNNSolution(){
}

int main(int argc, char** argv){

    // if(argc < 2){
    //     PrintUsage("Incorrect number of program arguments " + to_string(argc));
    //     exit(1);
    // }
    if(DEBUG){ printf("argc %d\n", argc);}

    // inputFile = argv[argc-1];
    if(DEBUG){ printf("input file %s\n", inputFile.c_str());}

    for (int i = 1; i < argc-1; i++){
    }

    algo=ALGO::KNN;
    if(algo == ALGO::KNN){
        ParseKNNInput(inputFile, false);
        if(DEBUG){ DisplayKNNInput();}
        // PrintKNNSolution();
    }

    return 0;
}