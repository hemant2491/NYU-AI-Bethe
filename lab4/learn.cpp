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
#include <unordered_set>
#include <deque>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <float.h>
#include <limits.h>
#include <sstream>
#include <cmath>

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

void KNN_ReadNeighborsFromFile(const string train_file, vector<Entry*>& neighbors, set<string>& labels_in_train, int& attrLength);
void KNN_DisplayInput(const vector<Entry*>& neighbors);
bool SortByDistance(pair<double, string> p1, pair<double, string> p2);
double KNN_CalculateDistance(Entry* e1, Entry* e2, int& attrLength, bool knn_UseEuclid);
string KNN_PredictLabel(Entry* test_entry, int k, const vector<Entry*>& neighbors, set<string>& labels_in_train, int& attrLength, bool knn_UseEuclid);
void KNN_ReadPointsAndPredictLabel(const string test_file_name, const vector<Entry*>& neighbors, int k, set<string>& labels_in_train, int& attrLength, bool knn_UseEuclid);


bool DEBUG = false;

void PrintUsage(string error){
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
    ALGO algo;
    MODE mode;
    bool verbose;
    int attrLength;
    int c_laplace = 0;
    bool knn_UseEuclid;
    set<string> labels_in_train;
    vector<Entry*> neighbors;
    


    // inputFile = argv[argc-1];
    inputFile = "tests/1_knn1.train.txt";
    if(DEBUG){ printf("input file %s\n", inputFile.c_str());}

    for (int i = 1; i < argc-1; i++){
    }

    verbose = false;
    train_file = "tests/1_knn1.train.txt";
    test_file = "tests/1_knn1.test.txt";
    attrLength = 0;

    algo=ALGO::KNN;
    k_knn = 3;
    knn_UseEuclid = true;
    if(algo == ALGO::KNN){
        KNN_ReadNeighborsFromFile(train_file, neighbors, labels_in_train, attrLength);
        if(DEBUG){ KNN_DisplayInput(neighbors);}
        KNN_ReadPointsAndPredictLabel(test_file, neighbors, k_knn, labels_in_train, attrLength, knn_UseEuclid);
    }

    return 0;
}

//##################### KNN Functions #####################

void KNN_ReadNeighborsFromFile(const string train_file, vector<Entry*>& neighbors, set<string>& labels_in_train, int& attrLength){
    ifstream fin;
    string line;
    if(DEBUG){ printf("parsing input file: %s\n", train_file.c_str());}

    try
    {
        fin.open(train_file);
    }
    catch(std::exception const& e)
    {
        printf("There was an error in opening input file %s: %s\n", train_file.c_str(), e.what());
        exit(1);
    }

    int entryCount = 0;
    
    while(getline(fin, line)){
        // Read a line from input file
        trim(line);
        if(DEBUG){ printf("Read line: '%s'\n", line.c_str());}
        if(line.empty()){ continue;}

        Entry* e = new Entry();
        auto label_position = line.rfind(",");
        e->label = line.substr(label_position+1, line.size()-1);
        labels_in_train.insert(e->label);
        entryCount++;

        stringstream s(line.substr(0, label_position));
        string token = "";
        while(getline(s, token, ',')){
            trim(token);
            e->attributes.push_back(stoi(token));
        }

        if (attrLength == 0){
            attrLength = e->attributes.size();
        } else if (e->attributes.size() != attrLength){
            printf("Error: inconsistent attribute length for entry of label %s at row %d\nexpected %d found %lu", 
                    token.c_str(), entryCount, attrLength, e->attributes.size());
            exit(1);
        }

        neighbors.push_back(e);
    }

    fin.close();
}

void KNN_DisplayInput(const vector<Entry*>& neighbors){

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


bool SortByDistance(pair<double, string> p1, pair<double, string> p2){
    return p1.first < p2.first;
}

double KNN_CalculateDistance(Entry* e1, Entry* e2, int& attrLength, bool knn_UseEuclid){
    double distance = 0;
    for (int i = 0; i < attrLength; i++){
        if(knn_UseEuclid){
            distance += pow(e1->attributes[i] - e2->attributes[i], 2);
        } else {
            distance += abs(e1->attributes[i] - e2->attributes[i]);
        }
    }
    return distance;
}

string KNN_PredictLabel(Entry* test_entry, int k, const vector<Entry*>& neighbors,
                 set<string>& labels_in_train, int& attrLength, bool knn_UseEuclid){
    
    string predicted_label = "";

    vector<pair<double, string>> distances;
    for(auto n = neighbors.begin(); n != neighbors.end(); n++){
        double distance = KNN_CalculateDistance(test_entry, *n, attrLength, knn_UseEuclid);
        distances.push_back(make_pair(distance, (*n)->label));
    }

    sort(distances.begin(), distances.end(), SortByDistance);

    unordered_map<string,int> votes;
    int maxVotes = 0;
    for (auto l : labels_in_train){
        votes[l] = 0;
    }
    for (int i = 0; i < k; i++){
        votes[distances[i].second]++;
        if(DEBUG){ printf("neighbor %d label %s\n", i, distances[i].second.c_str());}
    }
    for (auto l : labels_in_train){
        if(votes[l] > maxVotes){
            maxVotes = votes[l];
            predicted_label = l;
        }
    }

    return predicted_label;
}

void KNN_ReadPointsAndPredictLabel(const string test_file_name, const vector<Entry*>& neighbors, int k, 
                        set<string>& labels_in_train, int& attrLength, bool knn_UseEuclid){
    ifstream fin;
    string line;
    if(DEBUG){ printf("parsing input file: %s\n", test_file_name.c_str());}

    try
    {
        fin.open(test_file_name);
    }
    catch(std::exception const& e)
    {
        printf("There was an error in opening input file %s: %s\n", test_file_name.c_str(), e.what());
        exit(1);
    }

    // labels in training data + any new labels in test data
    set<string> all_labels = labels_in_train;
    unordered_map<string, int> predictions;
    unordered_map<string, int> true_predictions;
    unordered_map<string, int> ground_truth;

    for (auto l : labels_in_train){
        predictions[l] = 0;
        true_predictions[l] = 0;
        ground_truth[l] = 0;
    }
    
    while(getline(fin, line)){
        // Read a line from input file
        trim(line);
        if(DEBUG){ printf("Read line: '%s'\n", line.c_str());}
        if(line.empty()){ continue;}

        Entry* e = new Entry();
        auto label_position = line.rfind(",");
        string point_label = line.substr(label_position+1, line.size()-1);
        e->label = point_label;
        if(ground_truth.find(point_label) == ground_truth.end()){
            all_labels.insert(point_label);
            predictions[point_label] = 0;
            true_predictions[point_label] = 0;
            ground_truth[point_label] = 0;
        }

        stringstream s(line.substr(0, label_position));
        string token = "";
        while(getline(s, token, ',')){
            trim(token);
            e->attributes.push_back(stoi(token));
        }

        if (attrLength == 0){
            attrLength = e->attributes.size();
        } else if (e->attributes.size() != attrLength){
            printf("Error: inconsistent attribute length for entry of label %s\nexpected %d found %lu", 
                    token.c_str(), attrLength, e->attributes.size());
            exit(1);
        }

        string predicted_label = KNN_PredictLabel(e, k, neighbors, labels_in_train, attrLength, knn_UseEuclid);
        printf("want=%s got=%s\n", point_label.c_str(), predicted_label.c_str());

        predictions[predicted_label]++;
        if(point_label == predicted_label){
            true_predictions[predicted_label]++;
        }
        ground_truth[point_label]++;
    }

    for(auto l : all_labels){
        printf("Label=%s Precision=%d/%d Recall=%d/%d\n", l.c_str(), 
            true_predictions[l], predictions[l], true_predictions[l], ground_truth[l]);
    }

    fin.close();
}
