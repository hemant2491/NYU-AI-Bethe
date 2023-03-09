/*
* Lab2: Hemant Ramawat (hr2378)
* 
* Use ReadMe file for how to compile and run
* 
* Following below code structure from assignment
* 
* graph = parseInput(filename)
* clauses = graphConstraints(graph)
* assignments = dpll(clauses)
* solution = convertBack(assignments)
* print solution
* 
*/

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <set>

using namespace std;

bool VERBOSE = false;

class Graph {
    public:
        int N; //number of vertices
        set<string> nodes; //set of vertices
        unordered_map<string, vector<string>> adjL; //node adjacency list

        void UpdateVertexCount(){
            N = nodes.size();
        }

        void AddEdge(string x, string y){
            adjL[x].push_back(y);
            nodes.insert(y);
        }

        void AddNode(string x){
            nodes.insert(x);
        }

        void Print(){
            printf("Graph:\n\tNumber of nodes: %d\n\t", N);
            for (auto iter = nodes.begin(); iter != nodes.end(); iter++){
                printf("'%s' ", iter->c_str());
            }
            printf("\n\n");
            for (auto element : adjL){
                printf("\t%s:", element.first.c_str());
                for (auto neighboor : element.second){
                    printf(" %s", neighboor.c_str());
                }
                printf("\n");
            }
        }
};

// trim whitespace from start of string
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim whitespace from end of string
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends
static inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

void PrintUsage(string error){
    printf("%s\n",error.c_str());
    printf("Usage:\n");
    printf("./solver [-v] $ncolors $input-file\n");
    printf("\t* -v: an optional flag for verbose mode\n");
    printf("\t* $ncolors: the number of colors to solve for.  If 2 use R, G; if 3 RGB; 4 RGBY.\n");
    printf("\t* $input-file: a graph input file \n");
}

Graph ParseInput(string inputFileName){
    ifstream fin;
    string line;
    Graph graph;

    try
    {
        fin.open(inputFileName);
    }
    catch(std::exception const& e)
    {
        printf("There was an error in opening input file %s: %s\n", inputFileName.c_str(), e.what());
        exit(1);
    }

    while (fin) {
        // Read a line from input file
        getline(fin, line);

        // printf("%s\n", line.c_str());
        trim(line);
        // printf("%s\n", line.c_str());

        if(line.empty() || line.at(0) == '#'){ continue;}
        // printf("%s\n", line.c_str());

        int foundChildren = line.find(":");
        string parent = line.substr(0, foundChildren);
        trim(parent);
        graph.AddNode(parent);

        string childrenString = line.substr(foundChildren+1, line.length()-1);
        char c_childrenString[childrenString.length()+1];
        strcpy(c_childrenString, childrenString.c_str());

        char *child = strtok(c_childrenString, "[] ,");
        while (child != NULL) {
            graph.AddEdge(parent, child);
            child = strtok(NULL, "[] ,");
        }
    }

    graph.UpdateVertexCount();
    graph.Print();
    return graph;

}

// solver [-v] $ncolors $input-file
int main(int argc, char** argv){

    int ncolors;
    string ncolorsStr, inputFile;

    if(argc < 3 || argc > 4){
        PrintUsage("Incorrect number of program arguments " + to_string(argc));
    }

    if(argc == 4 && argv[1] == "-v"){
        VERBOSE = true;
    }

    if(argc == 4 && string(argv[1]) != "-v"){
        PrintUsage("Incorrect verbose program argument " + string(argv[1]));
    }

    if(argc == 3){
        ncolorsStr = argv[1];
        inputFile = argv[2];
    } else {
        ncolorsStr = argv[2];
        inputFile = argv[3];
    }

    try {
        ncolors = stoi(ncolorsStr);
    } catch(std::exception const& e) {
        PrintUsage("Incorrect $ncolors argument " + ncolorsStr);
        exit(1);
    }

    if (ncolors < 2 || ncolors > 4){
        PrintUsage("Incorrect $ncolors argument " + ncolorsStr);
        exit(1);
    }

    struct stat buff;
    if (stat(inputFile.c_str(), &buff) != 0){
        PrintUsage("Input file " + inputFile + " does not exist");
        exit(1);
    }

    ParseInput(inputFile);
    
    return 0;
}