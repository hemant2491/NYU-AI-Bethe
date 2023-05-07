#include "kmeans.h"

void KMEANS_ReCalculateCentroids(vector<Point2*>& centroids, unordered_map<int, vector<Point*>>& clusters, int attrLength){
    for (int i = 0; i < centroids.size(); i++){
        for (int j = 0; j < attrLength; j++){
            double initial_val = centroids[i]->attributes[j];
            double attr_val = 1 / clusters[i].size();
            for(Point* p : clusters[i]){
                attr_val *= p->attributes[j];
            }
            centroids[i]->attributes[j] = attr_val;
        }
    }
}

bool KMEANS_ReAssignClusters(vector<Point2*>& centroids, unordered_map<int, vector<Point*>>& clusters,
                int attrLength, bool useEuclid){

    bool changed = false;

    unordered_map<int, vector<Point*>> tmp_clusters;
    for(int i = 0; i < centroids.size(); i++){
        vector<Point*> tmp_v;
        clusters[i] = tmp_v;
    }

    for (int i = 0; i < centroids.size(); i++){
        for (Point* p : clusters[i]){
            int cluster_id = -1;
            double min_distance = INT_MAX;
            for (int i = 0; i < centroids.size(); i++){
                double distance = CalculateDistanceFromCentroid(centroids[i], p, attrLength, useEuclid);
                if (distance < min_distance){
                    min_distance = distance;
                    cluster_id = i;
                }
            }
            tmp_clusters[cluster_id].push_back(p);
            if (cluster_id != i){
                changed = true;
            }
        }
    }

    clusters = tmp_clusters;

    return changed;
}

void KMEANS_SOLVE(vector<Point2*>& centroids, const vector<Point*>& neighbors,
            int attrLength, bool verbose, bool useEuclid){

    unordered_map<int, vector<Point*>> clusters;
    for(int i = 0; i < centroids.size(); i++){
        vector<Point*> tmp_v;
        clusters[i] = tmp_v;
    }

    for (Point* p : neighbors){
        int cluster_id = -1;
        double min_distance = INT_MAX;
        for (int i = 0; i < centroids.size(); i++){
            double distance = CalculateDistanceFromCentroid(centroids[i], p, attrLength, useEuclid);
            if (distance < min_distance){
                min_distance = distance;
                cluster_id = i;
            }
        }
        clusters[cluster_id].push_back(p);
    }
    KMEANS_ReCalculateCentroids(centroids, clusters, attrLength);

    bool changed = true;

    while (changed){
        changed = KMEANS_ReAssignClusters(centroids, clusters, attrLength, useEuclid);
        if(changed){
            KMEANS_ReCalculateCentroids(centroids, clusters, attrLength);
        }
    }

    vector<pair<int, Point2*>> centroid_with_id;

    for (int i = 0; i < centroids.size(); i++){
        centroid_with_id.push_back(make_pair(i, centroids[i]));
    }

    sort(centroid_with_id.begin(), centroid_with_id.end(), SortByFirstAttribute);

    for (int i = 0; i < centroid_with_id.size(); i++){
        printf("C%d = {", i);
        bool first = true;
        set<string> cluster_labels;
        for (Point* p : clusters[centroid_with_id[i].first]){
            cluster_labels.insert(p->label);
        }
        for(auto l : cluster_labels){
            printf("%s", l.c_str());
            if(!first){
                printf(",");
            } else {
                first = false;
            }
        }
        printf("}\n");
    }

    for (int i = 0; i < centroid_with_id.size(); i++){
        printf("([");
        bool first = true;
        for(auto a : centroid_with_id[i].second->attributes){
            if (first){
                printf("%12lf", a);
                first = false;
            } else {
                printf(" %12lf", a);
            }
        }
        printf("])\n");
    }



}