/*
* Lab3: Hemant Ramawat(hr2378)
* 
* Use ReadMe file for how to compile and run
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
#include <float.h>
#include <limits.h>

using namespace std;

class Node{
public:
    string name = "";
    double reward = 0.0;
    double value = 0.0;
    double lastValue = 0.0;
    double p;    // alpha = 1-p
    bool isDecisionNode = false;
    bool isChanceNode = false;
    vector<string> adjL;
    vector<double> probs;

    Node(string name) {
        this->name = name;
    }
};

bool DEBUG = false;
string inputFile;
bool verbose = false;
bool minArg = false;
double df = 1.0;
double tol = 0.01;
int iter = 100;
bool readDf = false, readTol = false, readIter = false;
unordered_map<string, Node*> graph;
vector<string> allNodeSumbols;

void PrintUsage(string error){
    printf("%s\n",error.c_str());
    printf("Usage:\n");
    printf("\t./mdp [-min] [-df <discount_factor>] [-tol <tolerance>] [-iter <num_of_iterations>] <input-file.txt>\n\n");
    printf("Example:\n");
    printf("\tmdp -df .9 -tol 0.0001 some-input.txt\n");
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

void ParseInput(const string inputFileName){
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

    while(getline(fin, line)){
        // Read a line from input file
        trim(line);
        if(DEBUG){ printf("Read line: '%s'\n", line.c_str());}
        if(line.empty() || line.at(0) == '#'){ continue;}

        int foundReward = line.find("=");
        int foundChildren = line.find(":");
        int foundProbabilities = line.find("%");

        if (foundReward > -1){
            string nodeSymbol = line.substr(0, foundReward);
            trim(nodeSymbol);
            Node* node;
            if (graph.find(nodeSymbol) != graph.end()){
                node = graph[nodeSymbol];
            } else {
                node = new Node(nodeSymbol);
                graph[nodeSymbol] = node;
                allNodeSumbols.push_back(nodeSymbol);
            }

            double reward = 0.0;
            string rewardString = line.substr(foundReward+1, line.length()-1);

            trim(rewardString);
            try{
                reward = stod(rewardString);
            } catch(std::exception const& e){
                printf("Incorrect reward value %s for %s\n", rewardString.c_str(), nodeSymbol.c_str());
                exit(1);
            }
            node->reward = reward;

            if(DEBUG){ printf("parent '%s' reward %lf\n", node->name.c_str(), node->reward);}

        } else if (foundChildren > -1){
            string nodeSymbol = line.substr(0, foundChildren);
            trim(nodeSymbol);
            Node* node;
            if (graph.find(nodeSymbol) != graph.end()){
                node = graph[nodeSymbol];
            } else {
                node = new Node(nodeSymbol);
                graph[nodeSymbol] = node;
                allNodeSumbols.push_back(nodeSymbol);
            }

            string childrenString = line.substr(foundChildren+1, line.length()-1);
            char c_childrenString[childrenString.length()+1];
            strcpy(c_childrenString, childrenString.c_str());

            char *child = strtok(c_childrenString, "[] ,");
            while(child != NULL){
                node->adjL.push_back(string(child));
                child = strtok(NULL, "[] ,");
            }
            if(DEBUG){
                printf("parent '%s' child nodes", nodeSymbol.c_str());
                for (auto it = node->adjL.begin(); it != node->adjL.end(); it++){
                    printf(" '%s'", it->c_str());
                }
                printf("\n");
            }
        } else if(foundProbabilities > -1){
            string nodeSymbol = line.substr(0, foundProbabilities);
            trim(nodeSymbol);
            Node* node;
            if (graph.find(nodeSymbol) != graph.end()){
                node = graph[nodeSymbol];
            } else {
                node = new Node(nodeSymbol);
                graph[nodeSymbol] = node;
                allNodeSumbols.push_back(nodeSymbol);
            }

            string probString = line.substr(foundProbabilities+1, line.length()-1);
            char c_probString[probString.length()+1];
            strcpy(c_probString, probString.c_str());
            char *prob;

            try{
                prob = strtok(c_probString, "[] ,");
                while(prob != NULL){
                    node->probs.push_back(stod(prob));
                    prob = strtok(NULL, "[] ,");
                }
            } catch(std::exception const& e){
                printf("Incorrect probability value %s for %s\n", prob, nodeSymbol.c_str());
                exit(1);
            }

            if(DEBUG){
                printf("parent '%s' probabilities", nodeSymbol.c_str());
                for (auto it = node->probs.begin(); it != node->probs.end(); it++){
                    printf(" '%lf'", *it);
                }
                printf("\n");
            }
        }
    }

    sort(allNodeSumbols.begin(), allNodeSumbols.end());

    fin.close();
}

void ValidateGraph(){
    if(DEBUG){ printf("%s\n", __func__);}
    for(auto iter = graph.begin(); iter != graph.end(); iter++){
        Node* node = iter->second;

        if(DEBUG){ printf("validating node: %s\n", node->name.c_str());}

        if(node->adjL.size() < 1){
            node->isDecisionNode = false;
            continue;
        }

        if(node->adjL.size() > 0 && node->probs.size() == node->adjL.size()){
            if(DEBUG){ printf("chance node loop 1\n");}

            node->isChanceNode = true;
            node->isDecisionNode = false;

            double totalProb = 0.0;

            for (auto p = node->probs.begin(); p != node->probs.end(); p++){
                totalProb += *p;
            }
            if (totalProb != 1){
                printf("%s: Total sum of transitrion probabilities is not equal to 1\n", node->name.c_str());
                exit(1);
            }

        } else if (node->adjL.size() == 1){
            node->isChanceNode = true;
            node->isDecisionNode = false;
            node->probs.push_back(1.0);

        } else if(node->adjL.size() > 0 && node->probs.size() != node->adjL.size()){
            if(DEBUG){ printf("Decision node\n");}
            node->isDecisionNode = true;
            if(node->probs.size() == 0){
                node->p = 1.0;
            } else {
                node->p = node->probs[0];
            }
        } else if (node->adjL.size() == 0 && node->probs.size() != 0){
            printf("Terminal node %s has decision node or chance node probabilities\n", node->name.c_str());
            exit(1);
        }
    }
}

double ValueIteration(){
    if(DEBUG){ printf("%s\n", __func__);}

    double maxError = -DBL_MAX;

    for(auto entry : graph){
        Node* node = entry.second;

        if(node->isDecisionNode){
            if(DEBUG){ printf("Decision node '%s' reward %lf\n", node->name.c_str(), node->reward);}
            vector<double> possibleValues;

            double alphaP = (1.0 - node->p) / (node->adjL.size() - 1);

            for(int i = 0; i < node->adjL.size(); i++){
                double pi_val = 0.0;
                for (int j = 0; j< node->adjL.size(); j++){
                    if (i == j){
                        pi_val += node->p * graph[node->adjL[j]]->lastValue;
                    } else {
                        pi_val += alphaP * graph[node->adjL[j]]->lastValue;
                    }
                }
                possibleValues.push_back(pi_val);
            }

            if (minArg){
                node->value = *min_element (possibleValues.begin(), possibleValues.end());
            } else {
                node->value = *max_element (possibleValues.begin(), possibleValues.end());
            }

        } else {
            double pi_val = 0.0;
            auto cIter = node->adjL.begin();
            auto pIter = node->probs.begin();

            for(; cIter  != node->adjL.end(); cIter++, pIter++){
                pi_val += *pIter * graph[*cIter]->lastValue;
            }

            node->value = node->reward + df * pi_val;
        }

        double iterError = abs(node->value - node->lastValue);
        if (maxError < iterError){
            maxError = iterError;
        }
    }

    return maxError;
    
}

void PrintValues(){
    printf("\n");
    for (string nodeSym : allNodeSumbols){
        Node* node = graph[nodeSym];
        printf("%s=%.3lf ", nodeSym.c_str(), node->lastValue);
    }
    printf("\n");
}

void Solve(){
    if(DEBUG){ printf("%s\n", __func__);}
    for(int i = 0; i < iter; i++){
        if(DEBUG){ PrintValues();}
        double error = ValueIteration();

        if(i > 1 && error <= tol){ break;}

        for (auto entry : graph){
            Node* node = entry.second;
            node->lastValue = node->value;
        }
    }
}

void PrintSolution(){
    if(DEBUG){ printf("%s\n", __func__);}
    for (string nodeSym : allNodeSumbols){
        Node* node = graph[nodeSym];

        double value = minArg ? DBL_MAX : DBL_MIN;
        string symbol = "";

        if(node->isDecisionNode){
            printf("%s -> ", node->name.c_str());

            for (auto c : node->adjL){
                double cValue = graph[c]->lastValue;
                if (minArg && cValue < value){
                    value = cValue;
                    symbol = c;
                } else if (!minArg && cValue > value){
                    value = cValue;
                    symbol = c;
                }
            }

            printf("%s\n", symbol.c_str());
        }
    }
    
    PrintValues();
}

// mdp -df .9 -tol 0.0001 some-input.txt
int main(int argc, char** argv){

    if(argc < 2){
        PrintUsage("Incorrect number of program arguments " + to_string(argc));
        exit(1);
    }
    if(DEBUG){ printf("argc %d\n", argc);}

    inputFile = argv[argc-1];
    if(DEBUG){ printf("input file %s\n", inputFile.c_str());}

    for (int i = 1; i < argc-1; i++){
        try {
            if(readDf){
                df = stod(argv[i]);
                readDf = false;
                if(DEBUG){ printf("-df %lf\n",df);}
            } else if(readTol){
                tol = stod(argv[i]);
                readTol = false;
                if(DEBUG){ printf("-tol %lf\n",tol);}
            } else if(readIter){
                iter = stoi(argv[i]);
                readIter = false;
                if(DEBUG){ printf("-iter %d\n",iter);}
            }
        } catch(std::exception const& e){
            string wrongArgument = readDf ? "-df " : (readTol ? "-tol " : "-iter ");
            PrintUsage("Incorrect argument: " + wrongArgument + string(argv[i]));
            exit(1);
        }

        if(string(argv[i]) == "-min"){
            minArg = true;
        } else if(string(argv[i]) == "-df"){
            readDf = true;
        } else if(string(argv[i]) == "-tol"){
            readTol = true;
        } else if(string(argv[i]) == "-iter"){
            readIter = true;
        } else if(string(argv[i]) == "-v"){
            DEBUG = true;
        }
    }

    ParseInput(inputFile);
    
    ValidateGraph();

    Solve();

    PrintSolution();
    
    return 0;
}