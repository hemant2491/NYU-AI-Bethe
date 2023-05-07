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
#include "kmeans.h"

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
    ALGO algo = ALGO::BAYES;
    MODE mode;
    bool verbose = false;
    int attrLength = 0;
    int c_laplace = 0;
    bool useEuclid;
    set<string> labels_in_train;
    // vector of possible of values for each attribute
    vector<set<int>> attributes_in_train;
    vector<Point*> neighbors;
    //Point 2 class can have double values for attributes
    vector<Point2*> centroids;

    // inputFile = argv[argc-1];
    inputFile = "tests/1_knn1.train.txt";
    if(DEBUG){ printf("input file %s\n", inputFile.c_str());}

    for (int i = 1; i < argc-1; i++){
    }

    // train_file = "tests/1_knn1.train.txt";
    // test_file = "tests/1_knn1.test.txt";
    // train_file = "tests/4_ex1_train.csv";
    // test_file = "tests/4_ex1_test.csv";
    train_file = "tests/6_km1.txt";
    ReadNeighborsFromFile(train_file, neighbors, labels_in_train, attrLength, attributes_in_train);
    if(DEBUG){ DisplayInput(neighbors);}

    // algo=ALGO::KNN;
    if(algo == ALGO::KNN){
        k_knn = 3;
        verbose = true;


        useEuclid = true;
        KNN_ReadPointsAndPredictLabel(test_file, neighbors, k_knn, labels_in_train, attrLength, useEuclid, verbose);
    }

    algo = ALGO::KMEANS;
    // algo = ALGO::BAYES;
    if (algo == ALGO::BAYES){
        c_laplace = 1;
        verbose = true;


        unordered_map<string,int> label_counts;
        unordered_map<string,double> label_probabilities;

        /*
        * map of class to vector of map of attribute to its count
        * unordered_map< _label_ , vector<unordered_map< _attribute_ , _attribute_count_ >>>
        */
        unordered_map<string, vector<unordered_map<int,int>>> label_to_attribute_counts;

        /*
        * map of class to vector of map of attribute to its probability
        * unordered_map< _label_ , vector<unordered_map< _attribute_ , _attribute_probability_ >>>
        */
        unordered_map<string, vector<unordered_map<int,double>>> label_to_attribute_probabilities;

        //training step
        NB_CalculateProbabilities(neighbors, labels_in_train, attrLength, attributes_in_train, c_laplace, verbose,
                label_counts, label_probabilities, label_to_attribute_counts, label_to_attribute_probabilities);

        NB_ReadPointsAndPredictLabel(test_file, neighbors, labels_in_train, attrLength,
                attributes_in_train, c_laplace, verbose, label_counts,
                label_probabilities, label_to_attribute_counts, label_to_attribute_probabilities);
    }

    algo = ALGO::KMEANS;
    if (algo == ALGO::KMEANS){
        // (0,0) (200,200) (500,500)
        Point2* p1 = new Point2(); Point2* p2 = new Point2(); Point2* p3 = new Point2();
        p1->label = "_"; p1->attributes.push_back(0); p1->attributes.push_back(0);
        p2->label = "_"; p2->attributes.push_back(200); p2->attributes.push_back(200);
        p3->label = "_"; p3->attributes.push_back(500); p3->attributes.push_back(500);

        centroids.push_back(p1); centroids.push_back(p2); centroids.push_back(p3);

        KMEANS_SOLVE(centroids, neighbors, attrLength, verbose, useEuclid);
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

