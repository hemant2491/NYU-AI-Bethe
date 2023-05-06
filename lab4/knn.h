#pragma once
#include <vector>
#include <set>

#include "helper.h"

using namespace std;

bool SortByDistance(pair<double, string> p1, pair<double, string> p2);
double KNN_CalculateDistance(Entry* e1, Entry* e2, int& attrLength, bool knn_UseEuclid);
string KNN_PredictLabel(Entry* test_entry, int k, const vector<Entry*>& neighbors, set<string>& labels_in_train, int& attrLength, bool knn_UseEuclid);
void KNN_ReadPointsAndPredictLabel(const string test_file_name, const vector<Entry*>& neighbors, int k, set<string>& labels_in_train, int& attrLength, bool knn_UseEuclid);


