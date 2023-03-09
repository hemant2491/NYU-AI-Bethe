#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <fstream>
#include <climits>
#include <cstring>

using namespace std;


unordered_map<string, vector<string>> gameGraph;
unordered_set<string> leaves;
unordered_map<string, int> leafValues;

void VisitChildren(unordered_set<string> visited, string parent, string grandParent) {
    
    if (visited.count(parent) > 0) {
        cout << "cyclic graph at node " << parent.c_str() << endl;
        exit(1);
    } else {
        visited.emplace(parent);
        if (gameGraph.count(parent) > 0) {
            for (auto i : gameGraph[parent]) {
                VisitChildren(visited, i, parent);
            }
        } else {
            if (leaves.count(parent) < 1) {
                // child node "e" of "c" not found
                printf("child node \"%s\" of \"%s\" not found\n", parent.c_str(), grandParent.c_str());
                exit(1);
            }
        }
    }
}

void ValidateDAG() {
    unordered_set<string> visited;
    for (auto element : gameGraph) {
        visited.clear();
        VisitChildren(visited, element.first, "");
    }
 }

string ReadInputFile(string graphFile ) {
    
    string root = "";
    ifstream fin(graphFile);
    string line;

    try
    {
        fin.open(graphFile);
    }
    catch(std::exception const& e)
    {
        printf("There was an error in opening input file %s: %s\n", graphFile.c_str(), e.what());
        exit(1);
    }

    // if (fin.fail()) {
    //     printf("Input file %s does not exist\n", graphFile.c_str());
    //     exit(1);
    // }

    while (fin) {
        // Read a line from input file
        getline(fin, line);

        // cout << line << endl;
        
        int foundChildren = line.find(":");
        int foundLeaves = line.find("=");

        // printf("Delim position: Children %d Leaf %d\n", foundChildren, foundLeaves);
        // continue;

        if (foundChildren != -1) {
            string parent = line.substr(0, foundChildren);

            string childrenString = line.substr(foundChildren+1, line.length()-1);
            char c_childrenString[childrenString.length()+1];
            strcpy(c_childrenString, childrenString.c_str());

            char *token = strtok(c_childrenString, "[] ,");
            vector<string> children;
            while (token != NULL) {
                children.push_back(token);
                token = strtok(NULL, "[] ,");
            }
            if (gameGraph.empty()) { root = parent;}
            gameGraph[parent] = children;

            // printf("Parent %s Children", parent.c_str());
            // for (string child : children) {
            //     printf(" %s", child.c_str());
            // }
            // printf("\n");
        }

        if (foundLeaves != -1) {
            string leaf = line.substr(0, foundLeaves);
            string leafValStr = line.substr(foundLeaves+1, line.length()-1);
            int leafVal = stoi(leafValStr);
            leaves.emplace(leaf);
            leafValues[leaf] = leafVal;

            // printf("Leaf %s %d\n", leaf.c_str(), leafVal);
        }
    }

    fin.close();
    ValidateDAG();
    return root;
}

void LogOutput(bool isMax, string parent, string child, int val) {
    //Ex: max(a) chooses a2 for 2
    if (isMax) {
        printf("max(%s) chooses %s for %d\n", parent.c_str(), child.c_str(), val);
    } else {
        printf("min(%s) chooses %s for %d\n", parent.c_str(), child.c_str(), val);
    }
}

int Minimax(string currentPlayer, int maxLimit, bool isAbPruning, int alpha, int beta, bool isMaxPlayer, bool isRoot, bool isVerbose) {
    if (leaves.count(currentPlayer) > 0) {
        return leafValues[currentPlayer];
    }

    int minLimit = -maxLimit;

    if (isMaxPlayer) {
        int maxValue = INT_MIN;
        int currentValue = maxValue;
        string chosenChild = "";
        for (string child : gameGraph[currentPlayer]) {
            currentValue = Minimax(child, maxLimit, isAbPruning, alpha, beta, !isMaxPlayer, false, isVerbose);
            if (currentValue > maxValue) {
                maxValue = currentValue;
                chosenChild = child;
            }
            if (currentValue >= maxLimit) { break;}
            if (isAbPruning && currentValue > alpha) { alpha = currentValue;}
            if (isAbPruning && beta <= alpha) { 
                // printf("isAbPruning %s beta %d alpha %d\n", isAbPruning ? "true" : "false", beta, alpha);
                break;
            }
        }

        if (beta > alpha && (isRoot || isVerbose)) {
            LogOutput(isMaxPlayer, currentPlayer, chosenChild, maxValue);
        }
        return maxValue;
    } else {
        int minValue = INT_MAX;
        int currentValue = minValue;
        string chosenChild = "";
        for (string child : gameGraph[currentPlayer])
        {
            currentValue = Minimax(child, maxLimit, isAbPruning, alpha, beta, !isMaxPlayer, false, isVerbose);
            if (currentValue < minValue) {
                minValue = currentValue;
                chosenChild = child;
            }
            if (currentValue < minLimit) {
                // printf("currentValue %d minLimit %d\n", currentValue, minLimit);
                break;
            }
            if (isAbPruning && currentValue < beta) { beta = currentValue;}
            if (isAbPruning && beta <= alpha) {
                // printf("isAbPruning %s beta %d alpha %d\n", isAbPruning ? "true" : "false", beta, alpha);
                break;
            }
        }

        if (beta > alpha && (isRoot || isVerbose)) {
            LogOutput(isMaxPlayer, currentPlayer, chosenChild, minValue);
        }
        return minValue;
    }
}

int main(int argc, char** argv) {
    
    bool isVerbose = false;
    bool isAbPruning = false;
    int maxLimit = INT_MAX;
    bool isMaxPlayer = true;
    bool isRoot = true;
    string inputFile;
    // inputFile = "./tests/example2.txt";
    inputFile = string(argv[argc-1]);

    for (int i = 1; i<argc; i++) {
        string option(argv[i]);
        // cout << option.c_str() << " ";
        if ("-v" == option) { isVerbose = true;}
        if ("-ab" == option) { isAbPruning = true;}
        if ("min" == option) {isMaxPlayer == false;}
        if (i == argc-3) { maxLimit = stoi(argv[i]);}
    }
    // cout << endl;

    string root = ReadInputFile(inputFile);

    // printf("Running minimax with below parameters:\n isVerbose = %s,\n Alpha-Beta Pruning = %s,\n maxLimit = %d,\n isMaxPlayer = %s,\n root = %s,\n isRoot = %s\n", isVerbose ? "true" : "false", isAbPruning ? "true" : "false", maxLimit ,isMaxPlayer ? "true" : "false", root.c_str(), isRoot ? "true" : "false");

    Minimax(root, maxLimit, isAbPruning, INT_MIN, INT_MAX, isMaxPlayer, isRoot, isVerbose);

    return 0;
}