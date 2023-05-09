#include <iostream>
#include <string>
#include <sys/stat.h>
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
#include <vector>
#include <set>

#include "knn.h"

using namespace std;

#define EPSILON 1e-9

extern bool DEBUG;

string KNN_PredictLabel(Point* test_point, int k, const vector<Point*>& neighbors,
                 set<string>& labels_in_train, int& attrLength, bool useEuclid){
    
    string predicted_label = "";

    vector<pair<double, string>> distances;
    for(auto n = neighbors.begin(); n != neighbors.end(); n++){
        double distance = CalculateDistance(test_point, *n, attrLength, useEuclid);
        if (distance < EPSILON){
            distance = EPSILON;
        }
        distances.push_back(make_pair(distance, (*n)->label));
    }

    sort(distances.begin(), distances.end(), SortByDistance);

    unordered_map<string,double> votes;
    double maxVotes = 0.0;
    for (auto l : labels_in_train){
        votes[l] = 0;
    }
    for (int i = 0; i < k; i++){
        double weight = 1.0 / distances[i].first;
        votes[distances[i].second] += weight;
        if(DEBUG){ printf("neighbor %d label %s weight %.4lf\n", i, distances[i].second.c_str(), weight);}
    }
    if(DEBUG){ printf("Votes:\n");}
    for (auto l : labels_in_train){
        if(DEBUG){ printf("%s: %.4lf\n", l.c_str(), votes[l]);}
        if(votes[l] > maxVotes){
            maxVotes = votes[l];
            predicted_label = l;
        }
    }

    return predicted_label;
}

void KNN_ReadPointsAndPredictLabel(const string test_file_name, const vector<Point*>& neighbors, int k, 
                        set<string>& labels_in_train, int& attrLength, bool useEuclid, bool verbose){
    ifstream fin;
    string line;
    if(DEBUG){ printf("reading test file: %s\n", test_file_name.c_str());}

    try
    {
        fin.open(test_file_name);
    }
    catch(std::exception const& e)
    {
        printf("There was an error in opening test file %s: %s\n", test_file_name.c_str(), e.what());
        exit(1);
    }

    unordered_map<string, int> predictions;
    unordered_map<string, int> true_predictions;
    unordered_map<string, int> ground_truth;
    // labels in training data + any new labels in test data
    set<string> all_labels = labels_in_train;

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

        Point* e = new Point();
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
            printf("Error: inconsistent attribute length for point of label %s\nexpected %d found %lu", 
                    token.c_str(), attrLength, e->attributes.size());
            exit(1);
        }

        string predicted_label = KNN_PredictLabel(e, k, neighbors, labels_in_train, attrLength, useEuclid);
        if (verbose){
            printf("want=%s got=%s\n", point_label.c_str(), predicted_label.c_str());
        }

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
