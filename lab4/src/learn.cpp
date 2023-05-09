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
#include <fstream>
#include <cstring>
#include <algorithm>
#include <float.h>
#include <limits.h>
#include <sstream>
#include <math.h>
#include <unordered_map>

#define EPSILON 1e-9

using namespace std;

class Point{
public:
    string label = "";
    vector<int> attributes;
    int id = -1;
    Point(){}
};

//Point 2 class can have double values for attributes
class Point2{
public:
    string label = "";
    vector<double> attributes;
    Point2(){}
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

/*====================== KNN Functions ======================*/

string KNN_PredictLabel(Point* test_point, int k, const vector<Point*>& neighbors, set<string>& labels_in_train, int& attrLength, bool useEuclid);
void KNN_ReadPointsAndPredictLabel(const string test_file_name, const vector<Point*>& neighbors, int k, set<string>& labels_in_train, int& attrLength, bool useEuclid, bool verbose);
/*===================================================================*/


/*====================== Naive Bayes Functions ======================*/

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
            int c_laplace, bool verbose,
            unordered_map<string,int>& label_counts, unordered_map<string,double>& label_probabilities,
            unordered_map<string, vector<unordered_map<int,int>>>& label_to_attribute_counts,
            unordered_map<string, vector<unordered_map<int,double>>>& label_to_attribute_probabilities);
/*===================================================================*/


/*====================== KMeans Functions ======================*/

void KMEANS_ReCalculateCentroids(vector<Point2*>& centroids, unordered_map<int, vector<Point*>>& clusters, int attrLength);

bool KMEANS_ReAssignClusters(vector<Point2*>& centroids, unordered_map<int, vector<Point*>>& clusters,
                int attrLength, bool useEuclid);

void KMEANS_SOLVE(vector<Point2*>& centroids, const vector<Point*>& neighbors,
            int attrLength, bool verbose, bool useEuclid);
/*===================================================================*/

static void PrintUsage(string error){
    printf("%s\n",error.c_str());
    printf("Usage:\n");
    printf("\t./learn -train <training_file> [-test <test_file>] -K <nof_nearest_neighbors> -C <Laplacian_correction> [-v] [-d e2|manh] [[x0,y0], [x1,y2], ...]\n\n");
    printf("\t• -train: the training file\n");
    printf("\t• -test: the testing data file (not used in kMeans)\n");
    printf("\t• -K: if > 0 indicates to use kNN and also the value of K (if 0, do Naive Bayes')\n");
    printf("\t• -C: if > 0 indicates the Laplacian correction to use (0 means don't use one)\n");
    printf("\t• -v: [optional] verbose flag that outputs each predicted vs actual label\n");
    printf("\t• -d: mandatory flag for KMeans, possible values 'e2' or 'manh' indicating euclidean distance squared or manhattan distance to use\n");
    printf("\t• arguments: if a list of centroids is provided those should be used for kMeans\n");
    printf("\nNote: it is illegal for both K and C to be greater than 0\n");
    printf("\nExample:\n");
    printf("\t./learn -train tests/1_knn1.train.txt -test tests/1_knn1.test.txt -K 3 -v\n");
    printf("\t./learn -train tests/1_knn1.train.txt -test tests/1_knn1.test.txt -K 3 -v\n");
    printf("\t./learn -train tests/4_ex1_train.csv -test tests/4_ex1_test.csv -C 0\n");
    printf("\t./learn -train tests/4_ex1_train.csv -test tests/4_ex1_test.csv -C 1 -v\n");
    printf("\t./learn -train tests/6_km1.txt -d e2 0,0 200,200 500,500\n");
    printf("\t./learn -train tests/7_km2.txt -d manh 0,0,0 200,200,200 500,500,500\n");
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

static double CalculateDistance(Point* e1, Point* e2, int& attrLength, bool useEuclid){
    double distance = 0;
    for (int i = 0; i < attrLength; i++){
        if(useEuclid){
            distance += pow(e1->attributes[i] - e2->attributes[i], 2);
        } else {
            distance += abs(e1->attributes[i] - e2->attributes[i]);
        }
    }
    return distance;
}

static double CalculateDistanceFromCentroid(Point2* c1, Point* e2, int& attrLength, bool useEuclid){
    double distance = 0.0;
    for (int i = 0; i < attrLength; i++){
        if(useEuclid){
            distance += pow((c1->attributes[i] - ((double)e2->attributes[i])), 2);
        } else {
            distance += abs(c1->attributes[i] - ((double)e2->attributes[i]));
        }
    }
    return distance;
}

static bool SortByDistance(pair<double, string> p1, pair<double, string> p2){
    return p1.first < p2.first;
}

static bool SortByFirstAttribute(pair<int, Point2*> c1, pair<int, Point2*> c2){
    return c1.second->attributes[0] < c2.second->attributes[0];
}

static bool SortById(Point* p1, Point* p2){
    return p1->id < p2->id;
}


bool DEBUG = false;

void ReadNeighborsFromFile(const string train_file, vector<Point*>& neighbors, set<string>& labels_in_train, int& attrLength, vector<set<int>>& attributes_in_train);
void DisplayInput(const vector<Point*>& neighbors);


int main(int argc, char** argv){

    if(argc < 2){
        PrintUsage("Missing program arguments");
        exit(1);
    }
    if(DEBUG){ printf("argc %d\n", argc);}

    bool verbose = false;
    bool read_train_file = false;
    bool read_test_file = false;
    bool read_K = false;
    bool read_LC = false;
    bool read_d = false;
    bool read_centroids = false;
    string train_file;
    string test_file;
    ALGO algo = ALGO::BAYES;
    MODE mode;
    int k_knn = 0;
    int attrLength = 0;
    int c_laplace = 0;
    bool useEuclid = true;
    // All points from the Training data file
    vector<Point*> neighbors;
    // All labels in Traing data file
    set<string> labels_in_train;
    // vector of possible of values for each attribute
    vector<set<int>> attributes_in_train;
    /* Centroids for KMeans Algorithm */
    vector<Point2*> centroids;

    for (int i = 0; i < argc; i++){
        if(DEBUG) { printf("%s ", argv[i]);}
        try{
            if(read_train_file){
                train_file = argv[i];
                if(DEBUG){ printf("train file %s\n", train_file.c_str());}
                read_train_file = false;
                continue;
            } else if (read_test_file){
                test_file = argv[i];
                if(DEBUG){ printf("test file %s\n", test_file.c_str());}
                read_test_file = false;
                continue;
            } else if(read_K){
                k_knn = stoi(argv[i]);
                read_K = false;
                continue;
            } else if(read_LC){
                c_laplace = stoi(argv[i]);
                read_LC = false;
                continue;
            } else if(read_d){
                string tmp_d_str = argv[i];
                if(tmp_d_str == "e2"){
                    useEuclid = true;
                } else if(tmp_d_str == "manh"){
                    useEuclid = false;
                } else{
                    PrintUsage("Incorrect value for distance argument " + tmp_d_str);
                    exit(1);
                }
                read_d = false;
                continue;
            } else if(read_centroids){
                Point2* p = new Point2();
                p->label = "_";
                string tmp_ctr_str = argv[i];
                stringstream ss(tmp_ctr_str);
                string tmp_attr = "";
                while(getline(ss, tmp_attr, ',')){
                    trim(tmp_attr);
                    p->attributes.push_back(stoi(tmp_attr));
                }
                centroids.push_back(p);
            }
        } catch(std::exception const& e){
            string wrongArgument = read_K ? "-K " : (read_LC ? "-C " : (read_d ? "-d " : "centroid "));
            PrintUsage("Incorrect argument: " + wrongArgument + string(argv[i]));
            exit(1);
        }

        string arg_str = argv[i];
        if(arg_str == "-train"){
            read_train_file = true;
            continue;
        } else if (arg_str == "-test"){
            read_test_file = true;
            continue;
        } else if(arg_str == "-K"){
            read_K = true;
            continue;
        } else if(arg_str == "-C"){
            read_LC = true;
            continue;
        } else if(arg_str == "-v"){
            verbose = true;
            continue;
        } else if(arg_str == "-d"){
            read_d = true;
            read_centroids = true;
            algo = ALGO::KMEANS;
            continue;
        } else if(arg_str == "-debug"){
            DEBUG = true;
        }
    }
    if(DEBUG){ printf("\n");}

    if (k_knn > 0){
        algo = ALGO::KNN;
        useEuclid = true;
    }

    if(read_train_file || read_test_file || read_K || read_LC || read_d){
        PrintUsage("Missing Arguments");
        exit(1);
    } else if(train_file == ""){
        PrintUsage("Training data file not provided");
        exit(1);
    } else if(test_file == "" && algo != ALGO::KMEANS){
        PrintUsage("Test data file not provided");
        exit(1);
    } else if(k_knn > 0 and c_laplace > 0){
        PrintUsage("it is illegal for both K and C to be greater than 0");
        exit(1);
    }

    ReadNeighborsFromFile(train_file, neighbors, labels_in_train, attrLength, attributes_in_train);
    if(DEBUG){ DisplayInput(neighbors);}

    if(algo == ALGO::KNN){
        if(DEBUG){
            printf("Running KNN with\ntrain file %s test file %s k %d verbose %s\n",
                    train_file.c_str(),
                    test_file.c_str(),
                    k_knn,
                    (verbose ? "true" : "false"));
        }
        KNN_ReadPointsAndPredictLabel(test_file, neighbors, k_knn, labels_in_train, attrLength, useEuclid, verbose);

    } else if (algo == ALGO::BAYES){
        unordered_map<string,int> label_counts;
        unordered_map<string,double> label_probabilities;

        /*
        * map of class to vector of map of attribute to its count
        * unordered_map< _label_ , vector<unordered_map< _attribute_ , _attribute_count_ >>>
        */
        unordered_map<string, vector<unordered_map<int,int>>> label_to_attribute_counts;

        /*
        * map of class to vector of map of attribute to its probability
        * unordered_map< _label_ , vector<unordered_map< _attribute_ , _attribute_probability_ >>>
        */
        unordered_map<string, vector<unordered_map<int,double>>> label_to_attribute_probabilities;

        //training step
        NB_CalculateProbabilities(neighbors, labels_in_train, attrLength, attributes_in_train, c_laplace, verbose,
                label_counts, label_probabilities, label_to_attribute_counts, label_to_attribute_probabilities);

        NB_ReadPointsAndPredictLabel(test_file, neighbors, labels_in_train, attrLength,
                attributes_in_train, c_laplace, verbose, label_counts,
                label_probabilities, label_to_attribute_counts, label_to_attribute_probabilities);

    } else if (algo == ALGO::KMEANS){
        KMEANS_SOLVE(centroids, neighbors, attrLength, verbose, useEuclid);
    }

    return 0;
}


void ReadNeighborsFromFile(const string train_file, vector<Point*>& neighbors, set<string>& labels_in_train, int& attrLength, vector<set<int>>& attributes_in_train){
    ifstream fin;
    string line;
    if(DEBUG){ printf("Reading train file: %s\n", train_file.c_str());}

    try
    {
        fin.open(train_file);
    }
    catch(std::exception const& e)
    {
        printf("There was an error in opening train file %s: %s\n", train_file.c_str(), e.what());
        exit(1);
    }

    int pointCount = 0;
    
    while(getline(fin, line)){
        // Read a line from input file
        trim(line);
        if(DEBUG){ printf("Read line: '%s'\n", line.c_str());}
        if(line.empty()){ continue;}

        Point* e = new Point();
        auto label_position = line.rfind(",");
        e->label = line.substr(label_position+1, line.size()-1);
        labels_in_train.insert(e->label);
        pointCount++;

        stringstream s(line.substr(0, label_position));
        string token = "";
        while(getline(s, token, ',')){
            trim(token);
            e->attributes.push_back(stoi(token));
            e->id = pointCount;
        }

        if (attrLength == 0){
            attrLength = e->attributes.size();
        } else if (e->attributes.size() != attrLength){
            printf("Error: inconsistent attribute length for point of label %s at row %d\nexpected %d found %lu", 
                    token.c_str(), pointCount, attrLength, e->attributes.size());
            exit(1);
        }

        neighbors.push_back(e);
    }

    vector<set<int>> tmp(attrLength);
    for(Point* e : neighbors){
        vector<int>& v = e->attributes;
        for(int i = 0; i < attrLength; i++){
            tmp[i].insert(v[i]);
        }
    }
    attributes_in_train = tmp;

    fin.close();
}

void DisplayInput(const vector<Point*>& neighbors){

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

/*====================== KNN Functions ======================*/

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
/*===================================================================*/


/*====================== Naive Bayes Functions ======================*/
void NB_CalculateProbabilities(const vector<Point*>& neighbors, const set<string>& labels_in_train, int attrLength, 
            const vector<set<int>>& attributes_in_train, int c_laplace, bool verbose,
            unordered_map<string,int>& label_counts, unordered_map<string,double>& label_probabilities,
            unordered_map<string, vector<unordered_map<int,int>>>& label_to_attribute_counts,
            unordered_map<string, vector<unordered_map<int,double>>>& label_to_attribute_probabilities){

    for (auto l : labels_in_train){
        label_counts[l] = 0;
        label_probabilities[l] = 0.0;
        for(auto iter = attributes_in_train.begin(); iter != attributes_in_train.end(); iter++){
            unordered_map<int,int> label_attribute_counts;
            unordered_map<int,double> label_attribute_probs;
            for (int i : (*iter)){
                label_attribute_counts[i] = 0;
                label_attribute_probs[i] = 0.0;
            }
            label_to_attribute_counts[l].push_back(label_attribute_counts);
            label_to_attribute_probabilities[l].push_back(label_attribute_probs);
        }
    }

    for(Point* e : neighbors){
        label_counts[e->label]++;
        auto point_attr_iter = e->attributes.begin();
        auto label_attr_counts_map_vec_iter = label_to_attribute_counts[e->label].begin();
        for(; point_attr_iter != e->attributes.end(); point_attr_iter++, label_attr_counts_map_vec_iter++){
            (*label_attr_counts_map_vec_iter)[*point_attr_iter]++;
        }
    }

    for (auto l : labels_in_train){
        label_probabilities[l] = ((double) label_counts[l]) / neighbors.size();

        auto label_attr_counts_map_vec_iter = label_to_attribute_counts[l].begin();
        auto label_attr_probs_map_vec_iter = label_to_attribute_probabilities[l].begin();
        auto attr_value_vec_iter = attributes_in_train.begin();
        int attr_indx = 0;

        for (; label_attr_counts_map_vec_iter != label_to_attribute_counts[l].end(); 
            label_attr_counts_map_vec_iter++, label_attr_probs_map_vec_iter++, attr_value_vec_iter++, attr_indx++){
            
            for(auto attr_value : (*attr_value_vec_iter)){
                double numerator = ((double) ((*label_attr_counts_map_vec_iter)[attr_value] + c_laplace));
                double denominator = (double) (label_counts[l] + ((*label_attr_counts_map_vec_iter).size() * c_laplace));
                (*label_attr_probs_map_vec_iter)[attr_value] = ((double) numerator) / denominator;
                if(DEBUG){
                    printf("(debug)P(A%d=%d|C=%s) = (%d + %d) / (%d + (%lu * %d)) = %.6lf\n",
                    attr_indx, attr_value, l.c_str(),
                    (*label_attr_counts_map_vec_iter)[attr_value], c_laplace,
                    label_counts[l], (*label_attr_counts_map_vec_iter).size(), c_laplace,
                    (*label_attr_probs_map_vec_iter)[attr_value]);
                }
            }
        }
    }

}

string NB_PredictLabel(Point* test_point, const vector<Point*>& neighbors, const set<string>& labels_in_train, int& attrLength,
            const vector<set<int>>& attributes_in_train, int c_laplace, bool verbose,
            unordered_map<string,int>& label_counts, unordered_map<string,double>& label_probabilities,
            unordered_map<string, vector<unordered_map<int,int>>>& label_to_attribute_counts,
            unordered_map<string, vector<unordered_map<int,double>>>& label_to_attribute_probabilities){

    unordered_map<string, double> per_class_probabilities;

    string predicted_label = "";

    for (auto label : labels_in_train){
        double tmp_probability = label_probabilities[label];
        stringstream debug_str;
        if(DEBUG){ debug_str << "(debug)NB(C=" << label << ") = " << tmp_probability;}
        if(verbose){
            printf("P(C=%s) = [%d / %lu]\n", label.c_str(), label_counts[label], neighbors.size());
        }
        for (int attr_idx = 0; attr_idx < attrLength; attr_idx++){
            int attr_value = test_point->attributes[attr_idx];
            tmp_probability *= label_to_attribute_probabilities[label][attr_idx][attr_value];
            if (verbose){
                printf("P(A%d=%d | C=%s) = %d / %lu\n", attr_idx, attr_value, label.c_str(), (label_to_attribute_counts[label][attr_idx][attr_value]+c_laplace), (label_counts[label] + ((label_to_attribute_counts[label][attr_idx]).size()*c_laplace)));
            }
            if(DEBUG){ debug_str << " * " << label_to_attribute_probabilities[label][attr_idx][attr_value];}
        }
        if(DEBUG){
            debug_str << " = " << tmp_probability;
            printf("%s\n", debug_str.str().c_str());
        }
        per_class_probabilities[label] = tmp_probability;
    }

    double max_probability = 0.0;

    for (auto label : labels_in_train){
        double label_probability = per_class_probabilities[label];
        if (verbose){
            printf("NB(C=%s) = %.6lf\n", label.c_str(), label_probability);
        }
        if (label_probability > max_probability){
            max_probability = label_probability;
            predicted_label = label;
        }
    }

    return predicted_label;
}


void NB_ReadPointsAndPredictLabel(const string test_file_name, const vector<Point*>& neighbors,
            const set<string>& labels_in_train, int attrLength, const vector<set<int>>& attributes_in_train,
            int c_laplace, bool verbose,
            unordered_map<string,int>& label_counts, unordered_map<string,double>& label_probabilities,
            unordered_map<string, vector<unordered_map<int,int>>>& label_to_attribute_counts,
            unordered_map<string, vector<unordered_map<int,double>>>& label_to_attribute_probabilities){

    ifstream fin;
    string line;
    if(DEBUG){ printf("Reading test file: %s\n", test_file_name.c_str());}

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

        string predicted_label = NB_PredictLabel(e, neighbors, labels_in_train, attrLength, attributes_in_train, c_laplace,
                                            verbose, label_counts, label_probabilities, label_to_attribute_counts,
                                            label_to_attribute_probabilities);
        if(verbose ){
            if (point_label != predicted_label){
                printf("fail: got \"%s\" != want \"%s\"\n", predicted_label.c_str(), point_label.c_str());
            } else {
                printf("match: \"%s\"\n", predicted_label.c_str());
            }
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
/*===================================================================*/


/*====================== KMeans Functions ======================*/

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

    bool changed = true;

    while (changed){
        changed = KMEANS_ReAssignClusters(centroids, clusters, attrLength, useEuclid);
        if(changed){
            KMEANS_ReCalculateCentroids(centroids, clusters, attrLength);
        }
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

/*===================================================================*/