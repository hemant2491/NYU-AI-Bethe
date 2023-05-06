
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
    printf("\t./learn -train <input-file.txt>\n\n");
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

