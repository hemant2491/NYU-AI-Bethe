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
 

string ReadInputFile(string graphFile ) {
    
    string root = "";
    ifstream fin;
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

int Minimax(string currentPlayer, int maxVal, bool isAbPruning, int alpha, int beta, bool isMaxPlayer, bool isRoot, bool isVerbose) {
    if (leaves.count(currentPlayer) > 0) {
        return leafValues[currentPlayer];
    }

    if (isMaxPlayer) {
        int maxValue = INT_MIN;
        int currentValue = maxValue;
        string chosenChild = "";
        for (string child : gameGraph[currentPlayer]) {
            currentValue = Minimax(child, maxVal, isAbPruning, alpha, beta, !isMaxPlayer, false, isVerbose);
            if (currentValue > maxValue) {
                maxValue = currentValue;
                chosenChild = child;
            }
        }

        if (isRoot || isVerbose) {
            LogOutput(isMaxPlayer, currentPlayer, chosenChild, maxValue);
        }
        return maxValue;
    } else {
        int minValue = INT_MAX;
        int currentValue = minValue;
        string chosenChild = "";
        for (string child : gameGraph[currentPlayer])
        {
            currentValue = Minimax(child, maxVal, isAbPruning, alpha, beta, !isMaxPlayer, false, isVerbose);
            if (currentValue < minValue) {
                minValue = currentValue;
                chosenChild = child;
            }
        }

        if (isRoot || isVerbose) {
            LogOutput(isMaxPlayer, currentPlayer, chosenChild, minValue);
        }
        return minValue;
    }
}

int main(int argc, char** argv) {
    
    bool isVerbose = false;
    bool isAbPruning = false;
    int maxVal = INT_MAX;
    bool isRootMax = true;
    string inputFile;
    inputFile = "./tests/example1.txt";

    string root = ReadInputFile(inputFile);

    Minimax(root, maxVal, isAbPruning, INT_MAX, INT_MIN, isRootMax, true, isVerbose);

    return 0;
}