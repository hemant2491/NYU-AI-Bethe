#include "kmeans.h"
extern bool DEBUG;

void PrintCentroids(const vector<Point2*>& centroids){
    printf("Centroids(%lu)\n", centroids.size());
    for(int i = 0; i < centroids.size(); i++){
        printf("%d->", i);
        vector<double> attrs = centroids[i]->attributes;
        for(auto j = 0; j < attrs.size(); j++){
            printf(" %.4lf", attrs[j]);
        }
        printf("\n");
    }
}

void PrintClusterStats(unordered_map<int, vector<Point*>>& clusters){
    if(DEBUG){
        printf("Clusters map size: %lu\n", clusters.size());
        for (int i = 0; i < clusters.size(); i++){
            printf("Cluster %d: %lu\n", i, clusters[i].size());
        }
    }
}

void KMEANS_ReCalculateCentroids(vector<Point2*>& centroids, unordered_map<int, vector<Point*>>& clusters, int attrLength){
    for (int i = 0; i < centroids.size(); i++){
        if(clusters[i].size() < 1){ continue;}
        for (int j = 0; j < attrLength; j++){
            double attr_sum = 0.0 ;
            for(Point* p : clusters[i]){
                attr_sum += p->attributes[j];
            }
            centroids[i]->attributes[j] = ((double) attr_sum) / clusters[i].size();
        }
    }
    if(DEBUG){ PrintCentroids(centroids);}
}

bool KMEANS_ReAssignClusters(vector<Point2*>& centroids, unordered_map<int, vector<Point*>>& clusters,
                int attrLength, bool useEuclid){

    bool changed = false;

    unordered_map<int, vector<Point*>> tmp_clusters;
    for(int i = 0; i < clusters.size(); i++){
        vector<Point*> tmp_v;
        tmp_clusters[i] = tmp_v;
    }

    for (int i = 0; i < clusters.size(); i++){
        for (Point* p : clusters[i]){
            int new_cluster_id = i;
            double min_distance = INT_MAX;
            if(DEBUG){ printf("%s->", p->label.c_str());}
            for (int j = 0; j < centroids.size(); j++){
                double distance = CalculateDistanceFromCentroid(centroids[j], p, attrLength, useEuclid);
                if(DEBUG){ printf(" %d %.12lf", j, distance);}
                if (distance < min_distance){
                    min_distance = distance;
                    new_cluster_id = j;
                }
            }
            if(DEBUG){ printf("\n");}
            tmp_clusters[new_cluster_id].push_back(p);
            if (new_cluster_id != i){
                changed = true;
            }
        }
    }

    if(DEBUG){ PrintClusterStats(tmp_clusters);}

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
    if(DEBUG){ PrintCentroids(centroids);}

    for (Point* p : neighbors){
        int new_cluster_id = 0;
        double min_distance = INT_MAX;
        if(DEBUG){ printf("%s->", p->label.c_str());}
        for (int i = 0; i < centroids.size(); i++){
            double distance = CalculateDistanceFromCentroid(centroids[i], p, attrLength, useEuclid);
            if(DEBUG){ printf(" %d %.4lf", i, distance);}
            if (distance < min_distance){
                min_distance = distance;
                new_cluster_id = i;
            }
        }
        if(DEBUG){ printf("\n");}
        clusters[new_cluster_id].push_back(p);
    }

    if(DEBUG){ PrintClusterStats(clusters);}
    
    KMEANS_ReCalculateCentroids(centroids, clusters, attrLength);
    // if(DEBUG){ PrintClusterStats(clusters);}

    bool changed = true;

    while (changed){
        changed = KMEANS_ReAssignClusters(centroids, clusters, attrLength, useEuclid);
        // if(DEBUG){ PrintClusterStats(clusters);}
        if(changed){
            KMEANS_ReCalculateCentroids(centroids, clusters, attrLength);
        }
        // if(DEBUG){ PrintClusterStats(clusters);}
    }

    /*
    * new centorid data structure to sort cebtroids by first attribute
    * each pair in centorid vector is the centroid point and its original id
    * we need original id to match corresponding cluster
    */
    vector<pair<int, Point2*>> centroid_with_id;

    for (int i = 0; i < centroids.size(); i++){
        centroid_with_id.push_back(make_pair(i, centroids[i]));
    }

    sort(centroid_with_id.begin(), centroid_with_id.end(), SortByFirstAttribute);

    for (int i = 0; i < centroid_with_id.size(); i++){
        printf("C%d = {", i+1);
        bool first = true;
        vector<Point*> cluster_points = clusters[centroid_with_id[i].first];
        sort(cluster_points.begin(), cluster_points.end(), SortById);
        for(int j = 0; j < cluster_points.size(); j++){
            if(!first){
                printf(",%s", cluster_points[j]->label.c_str());
            } else {
                printf("%s", cluster_points[j]->label.c_str());
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
                printf("%.13lf", a);
                first = false;
            } else {
                printf(" %.13lf", a);
            }
        }
        printf("])\n");
    }



}