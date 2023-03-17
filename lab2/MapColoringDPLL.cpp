/*
* Lab2: Hemant Ramawat(hr2378)
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
        unordered_map<string, set<string>> adjL; //node adjacency list

        void UpdateVertexCount(){
            N = nodes.size();
        }

        void AddEdge(string x, string y){
            adjL[x].insert(y);
            nodes.insert(y);
        }

        void AddNode(string x){
            nodes.insert(x);
            adjL[x];
        }

        void Print(){
            printf("Graph:\n\tNumber of nodes: %d\n\t", N);
            for(auto iter = nodes.begin(); iter != nodes.end(); iter++){
                printf("'%s' ", iter->c_str());
            }
            printf("\n\n");
            for(auto element : adjL){
                printf("\t%s:", element.first.c_str());
                for(auto neighboor : element.second){
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
        Atom* next = NULL;
        Atom* previous = NULL;
        bool isIncluded = false;
        bool isNegated = false;

        Atom(string _sym){
            isSymbol = true;
            isIncluded = true;
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

        void SetNegated(bool b){
            isNegated = b;
        }

        void SetIncluded(bool b){
            isIncluded = b;
        }

        void Print(){
            if(isSymbol){
                printf("Sym: %s ", symbol.c_str());
            } else if(isOperation && type == OP_TYPE::AND){
                printf("Op: ^ ");
            } else if(isOperation && type == OP_TYPE::OR){
                printf("OP: v ");
            } else if(isOperation && type == OP_TYPE::NOT){
                printf("OP: ! ");
            } else {
                printf("Unknown Atom: isOperation %s isSymbol %s symbol %s ", isOperation ? "true" : "false", isSymbol ? "true" : "false", symbol.c_str());
            }
        }
        
};

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

    while(fin){
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
        while(child != NULL){
            graph.AddEdge(parent, child);
            graph.AddEdge(child, parent);
            child = strtok(NULL, "[] ,");
        }
    }

    graph.UpdateVertexCount();
    graph.Print();
    return graph;
}

inline vector<string> GetSymbolsWithColor(string node, int colorCount){
    
    const string colors[] = {"R", "G", "B", "Y"};
    vector<string> symbols;
    for(int i = 0; i<colorCount; i++){
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

void Visit(unordered_map<string, bool>& visited, string node, unordered_map<string, vector<string>>& nodesToSymbolsWithColorName, int colorCount, vector<vector<Atom>>& sentences, const set<string>& adjNodes){

    if(visited[node] == true){ return;}
    visited[node] = true;

    // if(graph.adjL.count(node) < 1){ return;}
    // set<string> adjNodes = graph.adjL[node];

    for(auto adjNode : adjNodes){
        // queue.push_back(adjNode);
        nodesToSymbolsWithColorName[adjNode] = GetSymbolsWithColor(adjNode, colorCount);
    }

    vector<string> parentAllColorSymbols = nodesToSymbolsWithColorName[node];
    vector<Atom> sentence;
    for(auto iter = parentAllColorSymbols.begin(); iter != parentAllColorSymbols.end(); ){
        Atom a(*iter);
        
        iter++;
        if(iter != parentAllColorSymbols.end()){
            Atom op = Atom(OP_TYPE::OR);
            a.SetNext(&op);
            sentence.push_back(a);
            sentence.push_back(op);
        } else {
            sentence.push_back(a);
        }
    }
    sentences.push_back(sentence);
    

    if(!adjNodes.empty()){
        for(int i = 0; i < colorCount; i++){
            for(auto iter = adjNodes.begin(); iter != adjNodes.end(); iter++){
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

                vector<Atom> sentence2;
                sentence2.push_back(not1);
                sentence2.push_back(parent);
                sentence2.push_back(or1);
                sentence2.push_back(not2);
                sentence2.push_back(adj);

                sentences.push_back(sentence2);
            }
        }
    }


    for(int i = 0; i < colorCount; i++){
        for(int j = 0; j < colorCount; j++){
            if(i==j){ continue;}
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

            vector<Atom> sentence3;
            sentence3.push_back(not1);
            sentence3.push_back(color1);
            sentence3.push_back(or1);
            sentence3.push_back(not2);
            sentence3.push_back(color2);

            sentences.push_back(sentence3);
        }
    }

    // while(!queue.empty()){
    //     string lNode = queue.front();
    //     // printf("Popping %s\n", lNode.c_str());
    //     queue.pop_front();
    //     Visit(visited, lNode, nodesToSymbolsWithColorName, colorCount, sentences, graph, queue);
    // }

}

tuple<vector<vector<Atom>>, set<string>> GraphConstraints(Graph graph, int colorCount){
    
    int N = graph.nodes.size();
    vector<vector<Atom>> sentences;
    unordered_map<string, bool> visited;
    unordered_map<string, vector<string>> nodesToSymbolsWithColorName;
    set<string> allSymbols;
    deque<string> queue;

    for(string node : graph.nodes){ visited[node] = false;}

    for(string node : graph.nodes){
        if(visited[node] == true){ continue;}
        // if(graph.adjL.count(node) < 1){ continue;}
        nodesToSymbolsWithColorName[node] = GetSymbolsWithColor(node, colorCount);
        Visit(visited, node, nodesToSymbolsWithColorName, colorCount, sentences, graph.adjL[node]);
    }

    // Negate symbol Atoms after '!'
    for(auto sentence : sentences){
        for(auto atom : sentence){
            if(atom.isOperation && atom.type == OP_TYPE::NOT){
                atom.next->SetNegated(true);
            }
        }
    }

    for(auto element : nodesToSymbolsWithColorName){
        for(auto sym : element.second){
            allSymbols.insert(sym);
        }
    }

    return {sentences, allSymbols};
}

void PrintSentences(vector<vector<Atom>> sentences, bool verbose){
    for(auto iter = sentences.begin(); iter != sentences.end(); iter++){
        for(auto iter2 = iter->begin(); iter2 != iter->end(); iter2++){
            if(iter2->isSymbol){
                printf("%s", iter2->symbol.c_str());
            } else {
                if(iter2->type == OP_TYPE::OR){
                    printf(" ");
                } else if(iter2->type == OP_TYPE::NOT){
                    printf("!");
                } else if(iter2->type == OP_TYPE::AND){
                    printf(" ^ ");
                } else {
                    printf("Incorrect Atom: ");
                    iter2->Print();
                }
            }
        }
        printf("\n");
    }
}

// tuple<vector<vector<Atom>>, bool> Propagate(Atom atom, vector<vector<Atom>>& sentences){
void Propagate(const string symbol, const bool isNegated, vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved){

    for(int i = 0; i < sentences.size(); i++){
        // vector<Atom> sentence = sentences[i];
        for(auto atomIter = sentences[i].begin(); atomIter != sentences[i].end(); atomIter++){
            if(atomIter->isSymbol && atomIter->symbol == symbol && atomIter->isNegated == isNegated){
                isSentenceSolved[i] = true;
                break;;
            } else if(atomIter->isSymbol && atomIter->symbol == symbol){
                atomIter->SetIncluded(false);
            }
        }
    }
}

bool IsSEmpty(vector<bool>& isSentenceSolved){
    for(auto solved : isSentenceSolved){
        if(!solved){ return false;}
    }
    return true;
}

bool isAnySentenceEmpty(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved){
    bool emptySentenceFound = false;

    for(int i = 0; i < sentences.size(); i++){
        if(!isSentenceSolved[i]){
            emptySentenceFound = true;
            for(auto atomIter = sentences[i].begin(); atomIter != sentences[i].end(); atomIter++){
                if(atomIter->isIncluded){
                    emptySentenceFound = false;
                    continue;
                }
            }
        }
    }
    return emptySentenceFound;
}

bool EasyCaseUnitLiteral(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, unordered_map<string,bool>& assignments, bool verbose){
    int includedSymbolCount = 0;
    bool unitLiteralFound = false;
    string unitSym;
    bool unitSymIsNegated;
    
    for(int i = 0; i < sentences.size(); i++){
        includedSymbolCount = 0;
        string tmpSym;
        bool tmpSymIsNegated;
        if(!isSentenceSolved[i]){
            for(auto atom : sentences[i]){
                if(atom.isSymbol && atom.isIncluded){
                    includedSymbolCount++;
                    tmpSym = atom.symbol;
                    tmpSymIsNegated = atom.isNegated;
                }
            }
            if (includedSymbolCount == 0){
                printf("Something went wrong. Found empty sentence. i = %d\n", i);
            }
        }
        if(includedSymbolCount == 1  && !unitLiteralFound){
            unitLiteralFound = true;
            unitSym = tmpSym;
            unitSymIsNegated = tmpSymIsNegated;
        } else if (includedSymbolCount == 1  && unitLiteralFound){
            if (tmpSym == unitSym && tmpSymIsNegated != unitSymIsNegated){
                return false;
            }
        }
    }

    if(unitLiteralFound){
        if(verbose){
            printf("easy case: unit literal %s\n", unitSym.c_str());
        }
        assignments[unitSym] = unitSymIsNegated;
        Propagate(unitSym, unitSymIsNegated, sentences, isSentenceSolved);
    }

    return true;
}

bool EasyCasePureLiteral(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, unordered_map<string,bool>& assignments, bool verbose){

    // map containing record of symbol name and if its positive and negated version is found
    // unordered_map<symbol_name, pair<poitive_found, negative_found>>
    unordered_map<string, pair<bool, bool>> symbolToSigns;
    bool pureLiteralFound = false;
    string pureSym;
    bool pureSymIsNegated = false;

    // Initialize symbol to signs map
    for(int i = 0; i < sentences.size(); i++){
        if(!isSentenceSolved[i]){
            for(auto atomIter = sentences[i].begin(); atomIter != sentences[i].end(); atomIter++){
                if(atomIter->isSymbol){
                    symbolToSigns[atomIter->symbol] = make_pair(false, false);
                }
            }
        }
    }

    // update positive or negated symbol found
    for(int i = 0; i < sentences.size(); i++){
        if(!isSentenceSolved[i]){
            for(auto atomIter = sentences[i].begin(); atomIter != sentences[i].end(); atomIter++){
                if(atomIter->isSymbol && !atomIter->isNegated){
                    symbolToSigns[atomIter->symbol].first = true;
                } else if(atomIter->isSymbol && atomIter->isNegated){
                    symbolToSigns[atomIter->symbol].second = true;
                } 
            }
        }
    }

    // find symol with only positive or negative value found
    for(auto element : symbolToSigns){
        if(element.second.first && !element.second.second){
            pureSymIsNegated = false;
            pureLiteralFound = true;
        } else if (!element.second.first && element.second.second){
            pureSymIsNegated = true;
            pureLiteralFound = true;
        }

        if (pureLiteralFound){
            pureSym = element.first;

            if(verbose){
                printf("easy case: pure literal %s=%s", pureSym.c_str(), pureSymIsNegated ? "false" : "true");
            }

            assignments[pureSym] = !pureSymIsNegated;

            Propagate(pureSym, pureSymIsNegated, sentences, isSentenceSolved);

            return true;
        }
    }
    
    return false;
}

// sentences = S
// assignments = A
bool DPLLSolver(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, unordered_map<string,bool>& assignments, bool verbose){
    // 1. check if S is empty
    if(IsSEmpty(isSentenceSolved)){ return true;}

    // 2. check if any sentence in S is empty
    if(isAnySentenceEmpty(sentences, isSentenceSolved)){ return false;}

    // 3. look for 'easy-case'
    // either a sentence with a single atom(aka unit-clause)
    // or a pure-literal(atom existing with always the same sign)
    // "propagate" the assignment, then back to start of solver
    if(EasyCaseUnitLiteral(sentences, isSentenceSolved, assignments, verbose)
        && EasyCasePureLiteral(sentences, isSentenceSolved, assignments, verbose)){

        return DPLLSolver(sentences, isSentenceSolved, assignments, verbose);
    }


    // 4. hard-case: 
    // if no 'easy-case', guess an atom, assign true, propagate and recursive call
    // If failure returned from recursive call try assigning False, propagate, recursive call to 1


    return true;

}

unordered_map<string,bool> DPLL(vector<vector<Atom>>& sentences, set<string>& allSymbols, bool verbose){
    // for(auto sym : allSymbols) Solve(sentences);
    vector<bool> isSentenceSolved(sentences.size(), false);
    unordered_map<string,bool> assignments;

    if(DPLLSolver(sentences, isSentenceSolved, assignments, verbose)){
        return assignments;
    } else {
        printf("NO VALID ASSIGNMENT\n");
    }
}

unordered_map<string, string> ConvertBack(unordered_map<string, bool>& assignments){
    
    set<string> stateAssigned;
    unordered_map<string, string> solution;
    
    for(auto assignment : assignments){
        if(assignment.second){
            string state = assignment.first.substr(0, assignment.first.size()-3);
            if(solution.count(state) < 1){
                string colorChar = assignment.first.substr(assignment.first.size()-1, assignment.first.size()-1);
                string color;
                if(colorChar == "R"){
                    color = "Red";
                } else if(colorChar == "G"){
                    color = "Green";
                } else if(colorChar == "B"){
                    color = "Blue";
                } else {
                    color = "Yellow";
                }
                solution[state] = color;
            } 
             
        }
    }
}

void PrintSolution(unordered_map<string,string>& solution){
    for(auto element : solution){
        printf("%s = %s\n", element.first.c_str(), element.second.c_str());
    }
}

// solver [-v] $ncolors $input-file
int main(int argc, char** argv){

    int ncolors;
    string ncolorsStr, inputFile;
    bool verbose = false;

    if(argc < 3 || argc > 4){
        PrintUsage("Incorrect number of program arguments " + to_string(argc));
        exit(1);
    }

    if(argc == 4 && argv[1] == "-v"){
        verbose = true;
    }

    if(argc == 4 && string(argv[1]) != "-v"){
        PrintUsage("Incorrect verbose program argument " + string(argv[1]));
        exit(1);
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
    } catch(std::exception const& e){
        PrintUsage("Incorrect $ncolors argument " + ncolorsStr);
        exit(1);
    }

    if(ncolors < 2 || ncolors > 4){
        PrintUsage("Incorrect $ncolors argument " + ncolorsStr);
        exit(1);
    }

    struct stat buff;
    if(stat(inputFile.c_str(), &buff) != 0){
        PrintUsage("Input file " + inputFile + " does not exist");
        exit(1);
    }

    Graph graph = ParseInput(inputFile);

    auto [sentences, allSymbols] = GraphConstraints(graph, ncolors);
    PrintSentences(sentences, verbose);

    unordered_map<string, bool> assignments = DPLL(sentences, allSymbols, verbose);
    unordered_map<string, string> solution = ConvertBack(assignments);
    PrintSolution(solution);
    
    return 0;
}