

#ifndef OK_GRASP_SOLUTION_H
#define OK_GRASP_SOLUTION_H
#include "vector"
#include "Graph.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

class Solution {

private:
    std::vector<std::vector<size_t>> routes;
    Graph graph;
    bool unacceptable;
public:
    std::vector<std::vector<size_t>> &getRoutes() ;
    std::vector<std::vector<size_t>> getRoutes() const ;

    bool is_unacceptable() const;

    void setUnacceptable();

public:
    static Solution load_solution(const std::string &result_file ,const std::string & input_file);

    size_t get_routes_number() const ;
    Solution swap(size_t route_number, size_t node_a, size_t node_b)const;
    Solution two_opt(size_t route_index, size_t start_index, size_t end_index) const;
    Solution relocation(size_t route, size_t target_route, size_t index, size_t target_index) const;
    bool is_legal(const std::vector<DataPoint> & data, size_t capacity, const DataPoint &depot) const;
    explicit Solution(const Graph  & graph);


};


#endif //OK_GRASP_SOLUTION_H
