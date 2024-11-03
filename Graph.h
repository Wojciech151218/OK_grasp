//
// Created by Wojciech on 01.11.2024.
//

#ifndef OK_GRASP_GRAPH_H
#define OK_GRASP_GRAPH_H
#include "vector"
#include "DataPoint.h"

class Graph {



    std::vector<std::vector<float >> edge_matrix;
    std::vector<float> depot_distances;

public:
    Graph();
    Graph(const std::vector<DataPoint> &data, const DataPoint &depot);
    float get_edge(size_t x, size_t y)const ;
    float get_distance_from_depot(size_t i)const ;
};


#endif //OK_GRASP_GRAPH_H
