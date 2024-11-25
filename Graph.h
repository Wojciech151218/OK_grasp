
#ifndef OK_GRASP_GRAPH_H
#define OK_GRASP_GRAPH_H
#include "vector"
#include "DataPoint.h"

class Graph {



    std::vector<std::vector<double >> edge_matrix;
    std::vector<double> depot_distances;

public:
    Graph();
    Graph(const std::vector<DataPoint> &data, const DataPoint &depot);
    double get_edge(size_t x, size_t y)const ;
    double get_distance_from_depot(size_t i)const ;
};


#endif //OK_GRASP_GRAPH_H
