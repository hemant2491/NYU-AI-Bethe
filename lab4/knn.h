#pragma once
#include <vector>
#include <set>

#include "helper.h"

using namespace std;

string KNN_PredictLabel(Point* test_point, int k, const vector<Point*>& neighbors, set<string>& labels_in_train, int& attrLength, bool useEuclid);
void KNN_ReadPointsAndPredictLabel(const string test_file_name, const vector<Point*>& neighbors, int k, set<string>& labels_in_train, int& attrLength, bool useEuclid, bool verbose);

