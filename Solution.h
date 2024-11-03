//
// Created by Wojciech on 02.11.2024.
//

#ifndef OK_GRASP_SOLUTION_H
#define OK_GRASP_SOLUTION_H
#include "vector"
#include "Graph.h"

class Solution {

private:
    std::vector<std::vector<size_t>> routes;
    Graph graph;
    bool unacceptable;
public:
    std::vector<std::vector<size_t>> &getRoutes() ;
    std::vector<std::vector<size_t>> getRoutes()const ;

    bool is_unacceptable() const;

    void setUnacceptable();

public:
    float get_routes_number() const ;
    Solution swap();
    Solution two_opt();
    Solution three_opt();
    Solution relocation();
    bool is_legal(const std::vector<DataPoint> & data, size_t capacity) const;
    explicit Solution(const Graph  & graph);


};


#endif //OK_GRASP_SOLUTION_H
