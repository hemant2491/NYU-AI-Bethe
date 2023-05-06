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
#include <deque>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <float.h>
#include <limits.h>
#include <sstream>
#include <cmath>

#include "helper.h"
#include "knn.h"
#include "naive_bayes.h"

using namespace std;

bool DEBUG = false;

void ReadNeighborsFromFile(const string train_file, vector<Point*>& neighbors, set<string>& labels_in_train, int& attrLength, vector<set<int>>& attributes_in_train);
void DisplayInput(const vector<Point*>& neighbors);


int main(int argc, char** argv){

    // if(argc < 2){
    //     PrintUsage("Incorrect number of program arguments " + to_string(argc));
    //     exit(1);
    // }
    if(DEBUG){ printf("argc %d\n", argc);}

    string inputFile;
    string train_file;
    string test_file;
    int k_knn;
    ALGO algo = ALGO::KMEANS;
    MODE mode;
    bool verbose = false;
    int attrLength = 0;
    int c_laplace = 0;
    bool knn_UseEuclid;
    set<string> labels_in_train;
    // vector of possible of values for each attribute
    vector<set<int>> attributes_in_train;
    vector<Point*> neighbors;

    // inputFile = argv[argc-1];
    inputFile = "tests/1_knn1.train.txt";
    if(DEBUG){ printf("input file %s\n", inputFile.c_str());}

    for (int i = 1; i < argc-1; i++){
    }

    // train_file = "tests/1_knn1.train.txt";
    train_file = "tests/4_ex1_train.csv";
    ReadNeighborsFromFile(train_file, neighbors, labels_in_train, attrLength, attributes_in_train);
    if(DEBUG){ DisplayInput(neighbors);}

    // algo=ALGO::KNN;
    k_knn = 3;
    knn_UseEuclid = true;
    // test_file = "tests/1_knn1.test.txt";
    if(algo == ALGO::KNN){
        KNN_ReadPointsAndPredictLabel(test_file, neighbors, k_knn, labels_in_train, attrLength, knn_UseEuclid);
    }

    
    algo = ALGO::BAYES;
    c_laplace = 1;
    verbose = true;
    test_file = "tests/4_ex1_test.csv";
    if (algo == ALGO::BAYES){
        NB_ReadPointsAndPredictLabel(test_file, neighbors, labels_in_train, attrLength, attributes_in_train, c_laplace, verbose);
    }

    return 0;
}


void ReadNeighborsFromFile(const string train_file, vector<Point*>& neighbors, set<string>& labels_in_train, int& attrLength, vector<set<int>>& attributes_in_train){
    ifstream fin;
    string line;
    if(DEBUG){ printf("Reading train file: %s\n", train_file.c_str());}

    try
    {
        fin.open(train_file);
    }
    catch(std::exception const& e)
    {
        printf("There was an error in opening train file %s: %s\n", train_file.c_str(), e.what());
        exit(1);
    }

    int pointCount = 0;
    
    while(getline(fin, line)){
        // Read a line from input file
        trim(line);
        if(DEBUG){ printf("Read line: '%s'\n", line.c_str());}
        if(line.empty()){ continue;}

        Point* e = new Point();
        auto label_position = line.rfind(",");
        e->label = line.substr(label_position+1, line.size()-1);
        labels_in_train.insert(e->label);
        pointCount++;

        stringstream s(line.substr(0, label_position));
        string token = "";
        while(getline(s, token, ',')){
            trim(token);
            e->attributes.push_back(stoi(token));
        }

        if (attrLength == 0){
            attrLength = e->attributes.size();
        } else if (e->attributes.size() != attrLength){
            printf("Error: inconsistent attribute length for point of label %s at row %d\nexpected %d found %lu", 
                    token.c_str(), pointCount, attrLength, e->attributes.size());
            exit(1);
        }

        neighbors.push_back(e);
    }

    vector<set<int>> tmp(attrLength);
    for(Point* e : neighbors){
        vector<int>& v = e->attributes;
        for(int i = 0; i < attrLength; i++){
            tmp[i].insert(v[i]);
        }
    }
    attributes_in_train = tmp;

    fin.close();
}

void DisplayInput(const vector<Point*>& neighbors){

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

