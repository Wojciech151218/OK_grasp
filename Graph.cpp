
#include "Graph.h"
#include "utils.h"

Graph::Graph(const std::vector<DataPoint> &data, const DataPoint &depot) {
    std::vector<std::vector<double>> result_matrix;
    std::vector<double> distance_result;
    auto data_size = data.size();

    for (size_t i = 0; i < data_size; ++i) {
        std::vector<double> row;
        let depot_distance = depot.get_distance(data[i]);
        distance_result.push_back(depot_distance);
        for (int j = 0; j < data_size; ++j) {
            if(i == j ){
                row.push_back(0.0f);
                continue;
            }
            auto distance = data[i].get_distance(data[j]);
            row.push_back(distance);
        }
        result_matrix.push_back(row);
    }
    edge_matrix = result_matrix;
    depot_distances = distance_result;
}



double Graph::get_edge(size_t x, size_t y)const {
    return edge_matrix[x][y];
}

Graph::Graph() {

}

double Graph::get_distance_from_depot(size_t i) const{
    return depot_distances[i];
}
