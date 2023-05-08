#pragma once
#include <string>
#include <algorithm>
#include <math.h>

using namespace std;

class Point{
public:
    string label = "";
    vector<int> attributes;
    int id = -1;
    Point(){}
};

//Point 2 class can have double values for attributes
class Point2{
public:
    string label = "";
    vector<double> attributes;
    Point2(){}
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

static double CalculateDistance(Point* e1, Point* e2, int& attrLength, bool useEuclid){
    double distance = 0;
    for (int i = 0; i < attrLength; i++){
        if(useEuclid){
            distance += pow(e1->attributes[i] - e2->attributes[i], 2);
        } else {
            distance += abs(e1->attributes[i] - e2->attributes[i]);
        }
    }
    return distance;
}

static double CalculateDistanceFromCentroid(Point2* c1, Point* e2, int& attrLength, bool useEuclid){
    double distance = 0.0;
    for (int i = 0; i < attrLength; i++){
        if(useEuclid){
            distance += pow((c1->attributes[i] - ((double)e2->attributes[i])), 2);
        } else {
            distance += abs(c1->attributes[i] - ((double)e2->attributes[i]));
        }
    }
    return distance;
}

static bool SortByDistance(pair<double, string> p1, pair<double, string> p2){
    return p1.first < p2.first;
}

static bool SortByFirstAttribute(pair<int, Point2*> c1, pair<int, Point2*> c2){
    return c1.second->attributes[0] < c2.second->attributes[0];
}

static bool SortById(Point* p1, Point* p2){
    return p1->id < p2->id;
}
