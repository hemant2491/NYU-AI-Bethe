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
#include <unordered_set>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <set>
#include <deque>

using namespace std;

enum OP_TYPE{NOT, AND, OR};

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

class Atom {
    public:
        bool isOperation = false;
        bool isSymbol = false;
        // bool notFlag;
        string symbol = "";
        OP_TYPE type;
        // char color;
        Atom* next;
        Atom* previous;

        Atom(string _sym){
            isSymbol = true;
            symbol = _sym;
        }

        Atom(OP_TYPE _type){
            isOperation = true;
            type = _type;
        }

        void SetNext(Atom* _next){
            next = _next;
        }

        void SetPrevious(Atom* _prev){
            previous = _prev;
        }

        void Print(){
            if (isSymbol) {
                printf("Sym: %s ", symbol.c_str());
            } else if (isOperation && type == OP_TYPE::AND){
                printf("Op: ^ ");
            } else if (isOperation && type == OP_TYPE::OR){
                printf("OP: v ");
            } else if (isOperation && type == OP_TYPE::NOT){
                printf("OP: ! ");
            } else {
                printf("Unknown Atom: isOperation %s isSymbol %s symbol %s ", isOperation ? "true" : "false", isSymbol ? "true" : "false", symbol.c_str());
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

inline vector<string> GetSymbolsWithColor(string node, int colorCount){
    
    const string colors[] = {"R", "B", "G", "Y"};
    vector<string> symbols;
    for (int i = 0; i<colorCount; i++){
        symbols.push_back(node + "_" + colors[i]);
    }
    return symbols;
}

/* 
For node WA with adjacent node NT and SA for 3 colors:
1. Color(WA,R) v Color(WA,G) v Color(WA,B)
2. Color(WA,R) =>¬[Color(NT,R) v Color(SA,R)] for each color
    => ¬Color(WA,R) v ¬Color(NT,R)
    => ¬Color(WA,R) v ¬Color(SA,R)
3. Color(WA,R) =>¬[Color(WA,G) v Color(WA,B)] for each color pair
    => ¬Color(WA,R) v ¬Color(WA,G)
    => ¬Color(WA,R) v ¬Color(WA,B)
 */

void Visit(unordered_map<string, bool>& visited, string node, unordered_map<string, vector<string>>& nodesToSymbolsWithColorName, int colorCount, vector<vector<Atom>>& clauses, Graph& graph, deque<string>& queue){

    visited[node] = true;

    if (graph.adjL.count(node) < 1) { return;}
    vector<string> adjNodes = graph.adjL[node];

    for (auto adjNode : adjNodes){
        queue.push_back(adjNode);
        nodesToSymbolsWithColorName[adjNode] = GetSymbolsWithColor(adjNode, colorCount);
    }

    vector<string> parentAllColorSymbols = nodesToSymbolsWithColorName[node];
    vector<Atom> clause;
    for (auto iter = parentAllColorSymbols.begin(); iter != parentAllColorSymbols.end(); ){
        Atom a(*iter);
        
        iter++;
        if(iter != parentAllColorSymbols.end()){
            Atom op = Atom(OP_TYPE::OR);
            a.SetNext(&op);
            clause.push_back(a);
            clause.push_back(op);
        } else {
            clause.push_back(a);
        }
    }
    clauses.push_back(clause);
    

    for (auto iter = adjNodes.begin(); iter != adjNodes.end(); iter++){
        for (int i = 0; i < colorCount; i++){
            Atom not1(OP_TYPE::NOT);
            Atom or1(OP_TYPE::OR);
            Atom not2(OP_TYPE::NOT);
            Atom parent(parentAllColorSymbols[i]);
            Atom adj(nodesToSymbolsWithColorName[*iter][i]);

            not1.SetNext(&parent);
            parent.SetNext(&or1);
            or1.SetNext(&not2);
            not2.SetNext(&adj);

            parent.SetPrevious(&not1);
            or1.SetPrevious(&parent);
            not2.SetPrevious(&or1);
            adj.SetPrevious(&not2);

            vector<Atom> clause2;
            clause2.push_back(not1);
            clause2.push_back(parent);
            clause2.push_back(or1);
            clause2.push_back(not2);
            clause2.push_back(adj);

            clauses.push_back(clause2);
        }
    }


    for (int i = 0; i < colorCount; i++){
        for (int j = i+1; j < colorCount; j++){
            Atom color1(parentAllColorSymbols[i]);
            Atom color2(parentAllColorSymbols[j]);
            Atom not1(OP_TYPE::NOT);
            Atom or1(OP_TYPE::OR);
            Atom not2(OP_TYPE::NOT);

            not1.SetNext(&color1);
            color1.SetNext(&or1);
            or1.SetNext(&not2);
            not2.SetNext(&color2);

            color1.SetPrevious(&not1);
            or1.SetPrevious(&color1);
            not2.SetPrevious(&or1);
            color2.SetPrevious(&not2);

            vector<Atom> clause3;
            clause3.push_back(not1);
            clause3.push_back(color1);
            clause3.push_back(or1);
            clause3.push_back(not2);
            clause3.push_back(color2);

            clauses.push_back(clause3);
        }
    }

    while(!queue.empty()){
        string lNode = queue.front();
        queue.pop_front();
        Visit(visited, lNode, nodesToSymbolsWithColorName, colorCount, clauses, graph, queue);
    }

}

vector<vector<Atom>> GraphConstraints(Graph graph, int colorCount){
    
    int N = graph.nodes.size();
    vector<vector<Atom>> clauses;
    unordered_map<string, bool> visited;
    unordered_map<string, vector<string>> nodesToSymbolsWithColorName;
    deque<string> queue;

    for (string node : graph.nodes) { visited[node] = false;}

    for (string node : graph.nodes) {
        if(visited[node] == true) { continue;}
        nodesToSymbolsWithColorName[node] = GetSymbolsWithColor(node, colorCount);
        Visit(visited, node, nodesToSymbolsWithColorName, colorCount, clauses, graph, queue);
    }

    return clauses;
}

void PrintClauses(vector<vector<Atom>> clauses, bool verbose){
    for (auto iter = clauses.begin(); iter != clauses.end(); iter++){
        for (auto iter2 = iter->begin(); iter2 != iter->end(); iter2++){
            if (iter2->isSymbol){
                printf("%s ", iter2->symbol.c_str());
            } else {
                if (iter2->type == OP_TYPE::OR){
                    printf(" ");
                } else if (iter2->type == OP_TYPE::NOT){
                    printf("!");
                } else if (iter2->type == OP_TYPE::AND){
                    printf("^ ");
                } else {
                    printf("Incorrect Atom: ");
                    iter2->Print();
                }
            }
        }
        printf("\n");
    }

}

vector<vector<Atom>> DPLL(vector<vector<Atom>> clauses){

}

void ConvertBack(){

}

// solver [-v] $ncolors $input-file
int main(int argc, char** argv){

    int ncolors;
    string ncolorsStr, inputFile;
    bool verbose = false;

    if(argc < 3 || argc > 4){
        PrintUsage("Incorrect number of program arguments " + to_string(argc));
    }

    if(argc == 4 && argv[1] == "-v"){
        verbose = true;
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

    Graph graph = ParseInput(inputFile);

    vector<vector<Atom>> clauses = GraphConstraints(graph, ncolors);
    PrintClauses(clauses, verbose);

    // vector<vector<Atom>> assignments = DPLL(clauses);

    // solution = convertBack(assignments)
    
    return 0;
}