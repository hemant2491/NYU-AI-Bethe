#pragma once
#include <set>

#include "helper.h"

using namespace std;

void NB_CalculateProbabilities(const vector<Point*>& neighbors, const set<string>& labels_in_train, int attrLength, 
            const vector<set<int>>& attributes_in_train, int c_laplace, bool verbose,
            unordered_map<string,int>& label_counts, unordered_map<string,double>& label_probabilities,
            unordered_map<string, vector<unordered_map<int,int>>>& label_to_attribute_counts,
            unordered_map<string, vector<unordered_map<int,double>>>& label_to_attribute_probabilities);

string NB_PredictLabel(Point* test_point, const vector<Point*>& neighbors, const set<string>& labels_in_train, int& attrLength,
            const vector<set<int>>& attributes_in_train, int c_laplace, bool verbose,
            unordered_map<string,int>& label_counts, unordered_map<string,double>& label_probabilities,
            unordered_map<string, vector<unordered_map<int,int>>>& label_to_attribute_counts,
            unordered_map<string, vector<unordered_map<int,double>>>& label_to_attribute_probabilities);

void NB_ReadPointsAndPredictLabel(const string test_file_name, const vector<Point*>& neighbors,
                const set<string>& labels_in_train, int attrLength, const vector<set<int>>& attributes_in_train,
                int c_laplace, bool verbose);
