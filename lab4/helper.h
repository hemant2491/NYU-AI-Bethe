#pragma once
#include <string>
#include <algorithm>

using namespace std;

class Entry{
public:
    string label = "";
    vector<int> attributes;
    Entry(){}
};

enum ALGO{
    BAYES,
    KNN,
    KMEANS
};

enum MODE{
    TRAIN,
    TEST
};

static void PrintUsage(string error){
    printf("%s\n",error.c_str());
    printf("Usage:\n");
    printf("\t./learn -train <training_file> [-test <test_file>] -K <nof_nearest_neighbors> -C <Laplacian_correction> [-v] [-d e2|manh] <input-file.txt> [[x0,y0], [x1,y2], ...]\n\n");
    printf("\t• -train: the training file\n");
    printf("\t• -test: the testing data file (not used in kMeans)\n");
    printf("\t• -K: if > 0 indicates to use kNN and also the value of K (if 0, do Naive Bayes')\n");
    printf("\t• -C: if > 0 indicates the Laplacian correction to use (0 means don't use one)\n");
    printf("\t• -v: [optional] verbose flag that outputs each predicted vs actual label\n");
    printf("\t• -d: possible values 'e2' or 'manh' indicating euclidean distance squared or manhattan distance to use\n");
    printf("\t• arguments: if a list of centroids is provided those should be used for kMeans\n");
    printf("\t• Note: it is illegal for both K and C to be greater than 0\n");
    printf("Example:\n");
    printf("\t./learn -train some-input.txt 0,500 200,200 1000,1000\n");
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
