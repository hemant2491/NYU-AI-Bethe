#pragma once
#include <vector>
#include <set>
#include <unordered_map>

#include "helper.h"

using namespace std;

void KMEANS_ReCalculateCentroids(vector<Point2*>& centroids, unordered_map<int, vector<Point*>>& clusters, int attrLength);

bool KMEANS_ReAssignClusters(vector<Point2*>& centroids, unordered_map<int, vector<Point*>>& clusters,
                int attrLength, bool useEuclid);

void KMEANS_SOLVE(vector<Point2*>& centroids, const vector<Point*>& neighbors,
            int attrLength, bool verbose, bool useEuclid);