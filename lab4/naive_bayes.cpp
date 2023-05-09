#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <set>
#include <string>
#include <unordered_map>

#include "knn.h"

using namespace std;

extern bool DEBUG;

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
