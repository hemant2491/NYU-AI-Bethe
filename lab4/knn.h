#pragma once
#include <vector>
#include <set>

#include "helper.h"

using namespace std;

bool SortByDistance(pair<double, string> p1, pair<double, string> p2);
double KNN_CalculateDistance(Point* e1, Point* e2, int& attrLength, bool knn_UseEuclid);
string KNN_PredictLabel(Point* test_point, int k, const vector<Point*>& neighbors, set<string>& labels_in_train, int& attrLength, bool knn_UseEuclid);
void KNN_ReadPointsAndPredictLabel(const string test_file_name, const vector<Point*>& neighbors, int k, set<string>& labels_in_train, int& attrLength, bool knn_UseEuclid);

