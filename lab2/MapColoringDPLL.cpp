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
#include <map>
#include <unordered_set>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <set>
#include <deque>

using namespace std;

enum OP_TYPE{NOT, AND, OR};
bool DEBUG = false;

class Graph {
    public:
        int N; //number of vertices
        set<string> nodes; //set of vertices
        map<string, set<string>> adjL; //node adjacency list

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

// Class for each atom in a sentence,
// could be a string(ex. NSW) or operation(ex. '^', 'v', '!')
class Atom {
    public:
        bool isOperation = false;
        bool isSymbol = false;
        string symbol = "";
        OP_TYPE type;
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

Graph ParseInput(const string inputFileName){
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
        trim(line);
        if(line.empty() || line.at(0) == '#'){ continue;}

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
    if(DEBUG){ graph.Print();}
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
1. Color(WA,R) v Color(WA,G) v Color(WA,B)                        --> Sentence type 1
2. Color(WA,R) =>¬[Color(NT,R) v Color(SA,R)] for each color      --> Sentence type 2
    => ¬Color(WA,R) v ¬Color(NT,R)
    => ¬Color(WA,R) v ¬Color(SA,R)
3. Color(WA,R) =>¬[Color(WA,G) v Color(WA,B)] for each color pair --> Sentence type 3
    => ¬Color(WA,R) v ¬Color(WA,G)
    => ¬Color(WA,R) v ¬Color(WA,B)
 */

void Visit(map<string, bool>& visited, string node, map<string, vector<string>>& nodesToSymbolsWithColorName, int colorCount, vector<vector<Atom>>& sentences, const set<string>& adjNodes){

    if(visited[node] == true){ return;}
    visited[node] = true;

    for(auto adjNode : adjNodes){
        nodesToSymbolsWithColorName[adjNode] = GetSymbolsWithColor(adjNode, colorCount);
    }

    vector<string> parentAllColorSymbols = nodesToSymbolsWithColorName[node];
    vector<Atom> sentence1;
    for(auto iter = parentAllColorSymbols.begin(); iter != parentAllColorSymbols.end(); ){
        Atom a(*iter);
        
        iter++;
        if(iter != parentAllColorSymbols.end()){
            Atom op = Atom(OP_TYPE::OR);
            sentence1.push_back(a);
            sentence1.push_back(op);
        } else {
            sentence1.push_back(a);
        }
    }
    sentences.push_back(sentence1);
    

    if(!adjNodes.empty()){
        for(int i = 0; i < colorCount; i++){
            for(auto iter = adjNodes.begin(); iter != adjNodes.end(); iter++){
                Atom not1(OP_TYPE::NOT);
                Atom or1(OP_TYPE::OR);
                Atom not2(OP_TYPE::NOT);
                Atom parent(parentAllColorSymbols[i]);
                Atom adj(nodesToSymbolsWithColorName[*iter][i]);

                parent.SetNegated(true);
                adj.SetNegated(true);

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

            color1.SetNegated(true);
            color2.SetNegated(true);

            vector<Atom> sentence3;
            sentence3.push_back(not1);
            sentence3.push_back(color1);
            sentence3.push_back(or1);
            sentence3.push_back(not2);
            sentence3.push_back(color2);

            sentences.push_back(sentence3);
        }
    }
}

tuple<vector<vector<Atom>>, set<string>> GraphConstraints(Graph graph, int colorCount){
    
    int N = graph.nodes.size();
    vector<vector<Atom>> sentences;
    map<string, bool> visited;
    map<string, vector<string>> nodesToSymbolsWithColorName;
    set<string> allSymbols;
    deque<string> queue;

    for(string node : graph.nodes){ visited[node] = false;}

    for(string node : graph.nodes){
        if(visited[node] == true){ continue;}
        nodesToSymbolsWithColorName[node] = GetSymbolsWithColor(node, colorCount);
        Visit(visited, node, nodesToSymbolsWithColorName, colorCount, sentences, graph.adjL[node]);
    }

    int i = 1;

    for (auto sentencesIter = sentences.begin(); sentencesIter != sentences.end(); sentencesIter++){
        Atom* previous;
        Atom* next;

         for(auto atomIter = sentencesIter->begin(); atomIter != sentencesIter->end(); atomIter++){
            if(atomIter->isSymbol){
                atomIter->SetIncluded(true);
            }

            if(atomIter != sentencesIter->end()){
                atomIter->SetNext(&(*std::next(atomIter,1)));
            }

            if(atomIter != sentencesIter->begin()){
                atomIter->SetPrevious(&(*std::prev(atomIter, 1)));
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



bool DPLLSolver(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, map<string,pair<bool,bool>>& assignments, bool verbose);

void CopySentences(vector<vector<Atom>>& sentences, vector<vector<Atom>>& sentencesCopy, vector<bool>& isSentenceSolved, bool verbose){

    for(auto sentencesIter = sentences.begin(); sentencesIter != sentences.end(); sentencesIter++){
        vector<Atom> sentenceCopy;
        for(auto atomIter = sentencesIter->begin(); atomIter != sentencesIter->end(); atomIter++){
            if (atomIter->isOperation){
                Atom a(atomIter->type);
                sentenceCopy.push_back(a);
            } else {
                Atom a(atomIter->symbol);
                a.SetIncluded(atomIter->isIncluded);
                a.SetNegated(atomIter->isNegated);
                sentenceCopy.push_back(a);
            }
        }
        sentencesCopy.push_back(sentenceCopy);
    }

    for(auto sentencesCopyIter = sentencesCopy.begin(); sentencesCopyIter != sentencesCopy.end(); sentencesCopyIter++){
        auto iter = sentencesCopyIter->begin();
        auto riter = sentencesCopyIter->rbegin();
        Atom* previous;
        Atom* next;

        for(;iter != sentencesCopyIter->end(); iter++, riter++){
            if(iter != sentencesCopyIter->begin()){
                iter->SetPrevious(previous);
            }
            if(riter != sentencesCopyIter->rbegin()){
                riter->SetNext(next);
            }
            previous = &(*iter);
            next = &(*riter);
        }
    }

}

void PrintSentences(vector<vector<Atom>> sentences, bool verbose, bool remainingOnly, vector<bool>& isSentenceSolved){
    auto sentencesIter = sentences.begin();
    auto solvedSentencesIter = isSentenceSolved.begin();

    for(; sentencesIter != sentences.end(); sentencesIter++, solvedSentencesIter++){
        if(remainingOnly && *solvedSentencesIter){
            continue;
        }
        bool startOfSentence = true;
        for(auto atomIter = sentencesIter->begin(); atomIter != sentencesIter->end(); atomIter++){
            if(startOfSentence && atomIter->isSymbol && !atomIter->isNegated && (!remainingOnly || atomIter->isIncluded)){
                printf("%s", atomIter->symbol.c_str());
                startOfSentence = false;
            } else if (startOfSentence && atomIter->isSymbol && atomIter->isNegated && (!remainingOnly || atomIter->isIncluded)){
                printf("!%s", atomIter->symbol.c_str());
                startOfSentence = false;
            } else if(!startOfSentence && atomIter->isSymbol && !atomIter->isNegated && (!remainingOnly || atomIter->isIncluded)){
                printf(" %s", atomIter->symbol.c_str());
            } else if (!startOfSentence && atomIter->isSymbol && atomIter->isNegated && (!remainingOnly || atomIter->isIncluded)){
                printf(" !%s", atomIter->symbol.c_str());
            }
            // } else {
                // if (DEBUG && remainingOnly){ printf("(%s)", atomIter->next->isSymbol ? atomIter->next->symbol.c_str() : "");}
                // if(atomIter->type == OP_TYPE::OR && (!remainingOnly || atomIter->previous->isIncluded)){
                //     printf(" ");
                // } else if(atomIter->type == OP_TYPE::NOT && (!remainingOnly || atomIter->next->isIncluded)){
                //     printf("!");
                // } else if(atomIter->type == OP_TYPE::AND && (!remainingOnly || (atomIter->previous->isIncluded && atomIter->next->isIncluded))){
                //     printf(" ^ "); 
                // } else {
                //     printf("Incorrect Atom: ");
                //     atomIter->Print();
                //     printf("\n");
                // }
            // }
        }
        printf("\n");
    }
}

void PrintAssignments(map<string,pair<bool,bool>>& assignments){
    // printf("%s Degug assignments size %d\n", __func__, assignments.size());
    for (auto assignment : assignments){
        if(assignment.second.first){
            printf("%s=%s\n", assignment.first.c_str(), assignment.second.second ? "true" : "false");
        } else {
            printf("%s=''\n", assignment.first.c_str());
        }
    }
}

// tuple<vector<vector<Atom>>, bool> Propagate(Atom atom, vector<vector<Atom>>& sentences){
void Propagate(const string symbol, const bool isNegated, vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, bool verbose){

    auto sentenceIter = sentences.begin();
    auto solvedSentencesIter = isSentenceSolved.begin();

    for(; sentenceIter != sentences.end(); sentenceIter++, solvedSentencesIter++){
        for(auto atomIter = sentenceIter->begin(); atomIter != sentenceIter->end(); atomIter++){
            if(atomIter->isSymbol && atomIter->symbol == symbol && atomIter->isNegated == isNegated){
                *solvedSentencesIter = true;
                break;
            } else if(atomIter->isSymbol && atomIter->symbol == symbol){
                atomIter->SetIncluded(false);
            }
        }
    }

    if(DEBUG){
        bool remainingSentencesOnly = true;
        PrintSentences(sentences, verbose, remainingSentencesOnly, isSentenceSolved);
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
    auto sentencesIter = sentences.begin();
    auto solvedSentencesIter = isSentenceSolved.begin();

    for(; sentencesIter != sentences.end(); sentencesIter++, solvedSentencesIter++){
        if(!*solvedSentencesIter){
            emptySentenceFound = true;
            for(auto atomIter = sentencesIter->begin(); atomIter != sentencesIter->end(); atomIter++){
                if(atomIter->isIncluded){
                    emptySentenceFound = false;
                    break;
                }
            }
        }
    }

    if(DEBUG && emptySentenceFound) { printf("%s DEBUG: Found empty sentence\n");}
    return emptySentenceFound;
}

bool EasyCaseUnitLiteral(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, map<string,pair<bool,bool>>& assignments, bool verbose){
    // if(DEBUG){ printf("%s DEBUG\n", __func__);}
    int includedSymbolCount = 0;
    bool unitLiteralFound = false;
    string unitSym;
    bool unitSymIsNegated;

    auto sentencesIter = sentences.begin();
    auto sentencesSolvedIter = isSentenceSolved.begin();
    
    for(; sentencesIter != sentences.end(); sentencesIter++, sentencesSolvedIter++){
        includedSymbolCount = 0;
        string tmpSym;
        bool tmpSymIsNegated;
        Atom* unitSymPtr;
        if(!*sentencesSolvedIter){
            for(auto atomIter = sentencesIter->begin(); atomIter != sentencesIter->end(); atomIter++){
                if(atomIter->isSymbol && atomIter->isIncluded){
                    includedSymbolCount++;
                    tmpSym = atomIter->symbol;
                    tmpSymIsNegated = atomIter->isNegated;
                    unitSymPtr = &(*atomIter);
                }
            }
            if (includedSymbolCount == 0){
                if(DEBUG){ printf("Something went wrong. Found empty sentence.\n");}
                return false;
            }
        }
        if(includedSymbolCount == 1  && !unitLiteralFound){
            unitLiteralFound = true;
            unitSym = tmpSym;
            unitSymIsNegated = tmpSymIsNegated;
            // break;
            // sentences[i][0].SetIncluded(false);
            // *sentencesSolvedIter = true;
        } 
        else if (includedSymbolCount == 1  && unitLiteralFound){
            if (tmpSym == unitSym && tmpSymIsNegated != unitSymIsNegated){
                if(DEBUG)
                {
                    printf("Found unit literal %s with opposite sign\n", tmpSym.c_str());
                }
                // return false;
            } 
        }
        // else if (tmpSym == unitSym){
        //     *sentencesSolvedIter = true;
        // }
    }

    if(unitLiteralFound){
        if(verbose){
            if(!unitSymIsNegated){
                printf("easy case: unit literal %s\n", unitSym.c_str());
            } else {
                printf("easy case: unit literal !%s\n", unitSym.c_str());
            }
        }
        assignments[unitSym] = make_pair(true,!unitSymIsNegated);
        if(DEBUG){ PrintAssignments(assignments);}
        Propagate(unitSym, unitSymIsNegated, sentences, isSentenceSolved, verbose);
        return true;
    }

    return false;
}

bool EasyCasePureLiteral(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, map<string,pair<bool,bool>>& assignments, bool verbose){

    // if(DEBUG){ printf("%s DEBUG\n", __func__);}

    // map containing record of symbol name and if its positive and negated version is found
    // map<symbol_name, pair<poitive_found, negative_found>>
    map<string, pair<bool, bool>> symbolToSigns;
    bool pureLiteralFound = false;
    string pureSym;
    bool pureSymIsNegated = false;
    auto sentencesIter = sentences.begin();
    auto solvedSentencesIter = isSentenceSolved.begin();

    // Initialize symbol to signs map
    for(; sentencesIter != sentences.end(); sentencesIter++, solvedSentencesIter++){
        if(!*solvedSentencesIter){
            for(auto atomIter = sentencesIter->begin(); atomIter != sentencesIter->end(); atomIter++){
                if(atomIter->isSymbol){
                    symbolToSigns[atomIter->symbol] = make_pair(false, false);
                }
            }
        }
    }

    sentencesIter = sentences.begin();
    solvedSentencesIter = isSentenceSolved.begin();
    // update positive or negated symbol found
    for(; sentencesIter != sentences.end(); sentencesIter++, solvedSentencesIter++){
        if(!*solvedSentencesIter){
            for(auto atomIter = sentencesIter->begin(); atomIter != sentencesIter->end(); atomIter++){
                if(atomIter->isSymbol && atomIter->isIncluded && !atomIter->isNegated){
                    symbolToSigns[atomIter->symbol].first = true;
                    // if(DEBUG){ printf("%s DEBUG: %s poistive\n", __func__, atomIter->symbol.c_str());}
                } else if(atomIter->isSymbol && atomIter->isIncluded && atomIter->isNegated){
                    symbolToSigns[atomIter->symbol].second = true;
                    // if(DEBUG){ printf("%s DEBUG: %s negative\n", __func__, atomIter->symbol.c_str());}
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
                printf("easy case: pure literal %s=%s\n", pureSym.c_str(), pureSymIsNegated ? "false" : "true");
            }

            assignments[pureSym] = make_pair(true,!pureSymIsNegated);

            Propagate(pureSym, pureSymIsNegated, sentences, isSentenceSolved, verbose);

            return true;
        }
    }

    return false;
}

bool EasyCase(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, map<string,pair<bool,bool>>& assignments, bool verbose){
    if(EasyCaseUnitLiteral(sentences, isSentenceSolved, assignments, verbose)){
        return true;
    }
    if(EasyCasePureLiteral(sentences, isSentenceSolved, assignments, verbose)){
        return true;
    }
    return false;
}



bool HardCase(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, map<string,pair<bool,bool>>& assignments, bool verbose){
    
    // Make copy of sentences, isSentenceSolved and assignments
    // before guessing in hard case for easy roll back
    vector<bool> isSentenceSolvedCopy = isSentenceSolved;
    map<string,pair<bool,bool>> assignmentsCopy = assignments;
    vector<vector<Atom>> sentencesCopy;
    CopySentences(sentences, sentencesCopy, isSentenceSolved, verbose);

    string symbol;
    bool isNegated;

    for (auto assignment : assignments){
        if (!assignment.second.first){
            symbol = assignment.first;
            break;
        }
    }

    if(verbose){
        printf("hard case: guess %s=true\n", symbol.c_str());
    }

    // assignments[symbol] = make_pair(true, true);
    assignments[symbol].first = true;
    assignments[symbol].second = true;

    // first try with true i.e. isNegated = false
    Propagate(symbol, false, sentences, isSentenceSolved, verbose);
    if (DPLLSolver(sentences, isSentenceSolved, assignments, verbose)){
        return true;
    }

    // roll back changes
    isSentenceSolved = isSentenceSolvedCopy;
    assignments = assignmentsCopy;
    sentences = sentencesCopy;

    if(verbose){
        printf("contradiction: backtrack guess %s=false\n", symbol.c_str());
    }
    // assignments[symbol] = make_pair(true, false);
    assignments[symbol].first = true;
    assignments[symbol].second = false;

    // now try with false i.e. isNegated = true;
    Propagate(symbol, true, sentences, isSentenceSolved, verbose);
    if (DPLLSolver(sentences, isSentenceSolved, assignments, verbose)){
        return true;
    }

    return false;
}

// sentences = S
// assignments = A
bool DPLLSolver(vector<vector<Atom>>& sentences, vector<bool>& isSentenceSolved, map<string,pair<bool,bool>>& assignments, bool verbose){
    // 1. check if S is empty
    if(IsSEmpty(isSentenceSolved)){ return true;}
    else { 
        if(DEBUG){ printf("DEBUG: S Not Empty\n", __func__);}
    }

    // 2. check if any sentence in S is empty
    if(isAnySentenceEmpty(sentences, isSentenceSolved)){ return false;}

    // 3. look for 'easy-case'
    // either a sentence with a single atom(aka unit-clause)
    // or a pure-literal(atom existing with always the same sign)
    // "propagate" the assignment, then back to start of solver
    if(EasyCase(sentences, isSentenceSolved, assignments, verbose)){
        return DPLLSolver(sentences, isSentenceSolved, assignments, verbose);
    }

    // 4. hard-case: 
    // if no 'easy-case', guess an atom, assign true, propagate and recursive call
    // If failure returned from recursive call try assigning False, propagate, recursive call to 1
    if (HardCase(sentences, isSentenceSolved, assignments, verbose)){
        return true;
    }

    return false;
}

map<string,pair<bool,bool>> DPLL(vector<vector<Atom>>& sentences, set<string>& allSymbols, bool verbose){

    vector<bool> isSentenceSolved(sentences.size(), false);
    map<string,pair<bool,bool>> assignments;
    vector<vector<Atom>> sentencesCopy;
    CopySentences(sentences, sentencesCopy, isSentenceSolved, verbose);

    if(DEBUG){
        bool remainingSentencesOnly = true;
        PrintSentences(sentencesCopy, verbose, remainingSentencesOnly, isSentenceSolved);
    }

    for (string symbol : allSymbols){
        assignments[symbol] = make_pair(false,false);
    }
    // if(DEBUG){ PrintAssignments(assignments);}

    if(DPLLSolver(sentencesCopy, isSentenceSolved, assignments, verbose)){
        for (auto assignment : assignments){
            if (!assignment.second.first){
                assignment.second.first = true;
                assignment.second.second = false;
            }
        }
        return assignments;
    } else {
        printf("NO VALID ASSIGNMENT\n");
        exit(1);
    }
}

void PrintSolution(map<string,string>& solution){
    for(auto element : solution){
        printf("%s = %s\n", element.first.c_str(), element.second.c_str());
    }
}

map<string, string> ConvertBack(map<string,pair<bool,bool>>& assignments){
    
    set<string> stateAssigned;
    map<string, string> solution;
    
    for(auto assignmentIter = assignments.begin(); assignmentIter != assignments.end(); assignmentIter++){
        if(assignmentIter->second.first && assignmentIter->second.second){
            string state = assignmentIter->first.substr(0, assignmentIter->first.size()-2);
            // if(DEBUG){ printf("%s DEBUG: %s\n", __func__, state.c_str());}
            if(solution.count(state) < 1){
                string colorChar = assignmentIter->first.substr(assignmentIter->first.size()-1, assignmentIter->first.size()-1);
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

    return solution;
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

    if(argc == 4 && string(argv[1]) == "-v"){
        verbose = true;
        // printf("verbose = true\n");
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

    map<string,pair<bool,bool>> assignments = DPLL(sentences, allSymbols, verbose);
    if (DEBUG){ PrintAssignments(assignments);}

    map<string, string> solution = ConvertBack(assignments);
    PrintSolution(solution);
    
    return 0;
}