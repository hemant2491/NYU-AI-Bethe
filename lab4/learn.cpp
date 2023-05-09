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

#include "helper.h"
#include "knn.h"
#include "naive_bayes.h"
#include "kmeans.h"

using namespace std;

bool DEBUG = false;

void ReadNeighborsFromFile(const string train_file, vector<Point*>& neighbors, set<string>& labels_in_train, int& attrLength, vector<set<int>>& attributes_in_train);
void DisplayInput(const vector<Point*>& neighbors);


int main(int argc, char** argv){

    if(argc < 2){
        PrintUsage("Missing program arguments");
        exit(1);
    }
    if(DEBUG){ printf("argc %d\n", argc);}

    bool verbose = false;
    bool read_train_file = false;
    bool read_test_file = false;
    bool read_K = false;
    bool read_LC = false;
    bool read_d = false;
    bool read_centroids = false;
    string train_file;
    string test_file;
    ALGO algo = ALGO::BAYES;
    MODE mode;
    int k_knn = 0;
    int attrLength = 0;
    int c_laplace = 0;
    bool useEuclid = true;
    // All points from the Training data file
    vector<Point*> neighbors;
    // All labels in Traing data file
    set<string> labels_in_train;
    // vector of possible of values for each attribute
    vector<set<int>> attributes_in_train;
    /* Centroids for KMeans Algorithm*/
    vector<Point2*> centroids;

    for (int i = 0; i < argc; i++){
        if(DEBUG) { printf("%s ", argv[i]);}
        try{
            if(read_train_file){
                train_file = argv[i];
                if(DEBUG){ printf("train file %s\n", train_file.c_str());}
                read_train_file = false;
                continue;
            } else if (read_test_file){
                test_file = argv[i];
                if(DEBUG){ printf("test file %s\n", test_file.c_str());}
                read_test_file = false;
                continue;
            } else if(read_K){
                k_knn = stoi(argv[i]);
                read_K = false;
                continue;
            } else if(read_LC){
                c_laplace = stoi(argv[i]);
                read_LC = false;
                continue;
            } else if(read_d){
                string tmp_d_str = argv[i];
                if(tmp_d_str == "e2"){
                    useEuclid = true;
                } else if(tmp_d_str == "manh"){
                    useEuclid = false;
                } else{
                    PrintUsage("Incorrect value for distance argument " + tmp_d_str);
                    exit(1);
                }
                read_d = false;
                continue;
            } else if(read_centroids){
                Point2* p = new Point2();
                p->label = "_";
                string tmp_ctr_str = argv[i];
                stringstream ss(tmp_ctr_str);
                string tmp_attr = "";
                while(getline(ss, tmp_attr, ',')){
                    trim(tmp_attr);
                    p->attributes.push_back(stoi(tmp_attr));
                }
                centroids.push_back(p);
            }
        } catch(std::exception const& e){
            string wrongArgument = read_K ? "-K " : (read_LC ? "-C " : (read_d ? "-d " : "centroid "));
            PrintUsage("Incorrect argument: " + wrongArgument + string(argv[i]));
            exit(1);
        }

        string arg_str = argv[i];
        if(arg_str == "-train"){
            read_train_file = true;
            continue;
        } else if (arg_str == "-test"){
            read_test_file = true;
            continue;
        } else if(arg_str == "-K"){
            read_K = true;
            continue;
        } else if(arg_str == "-C"){
            read_LC = true;
            continue;
        } else if(arg_str == "-v"){
            verbose = true;
            continue;
        } else if(arg_str == "-d"){
            read_d = true;
            read_centroids = true;
            algo = ALGO::KMEANS;
            continue;
        } else if(arg_str == "-debug"){
            DEBUG = true;
        }
    }
    if(DEBUG){ printf("\n");}

    if (k_knn > 0){
        algo = ALGO::KNN;
        useEuclid = true;
    }

    if(read_train_file || read_test_file || read_K || read_LC || read_d){
        PrintUsage("Missing Arguments");
        exit(1);
    } else if(train_file == ""){
        PrintUsage("Training data file not provided");
        exit(1);
    } else if(test_file == "" && algo != ALGO::KMEANS){
        PrintUsage("Test data file not provided");
        exit(1);
    } else if(k_knn > 0 and c_laplace > 0){
        PrintUsage("it is illegal for both K and C to be greater than 0");
        exit(1);
    }

    ReadNeighborsFromFile(train_file, neighbors, labels_in_train, attrLength, attributes_in_train);
    if(DEBUG){ DisplayInput(neighbors);}

    if(algo == ALGO::KNN){
        if(DEBUG){
            printf("Running KNN with\ntrain file %s test file %s k %d verbose %s\n",
                    train_file.c_str(),
                    test_file.c_str(),
                    k_knn,
                    (verbose ? "true" : "false"));
        }
        KNN_ReadPointsAndPredictLabel(test_file, neighbors, k_knn, labels_in_train, attrLength, useEuclid, verbose);

    } else if (algo == ALGO::BAYES){
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

    } else if (algo == ALGO::KMEANS){
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
            e->id = pointCount;
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

