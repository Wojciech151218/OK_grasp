//
// Created by Wojciech on 01.11.2024.
//

#ifndef OK_GRASP_PROBLEM_H
#define OK_GRASP_PROBLEM_H
#include "vector"
#include "DataPoint.h"
#include "FleetProperties.h"
#include "Graph.h"
#include "Solution.h"
#include "utils.h"


class Problem {
private:


public:
    Problem(std::vector<DataPoint> _data, const FleetProperties &fleetProperties, const DataPoint &depot);
    Solution solve_grasp(size_t epochs, size_t rcl_size, float threshold) const;
    float get_cost_function(const Solution & solution) const;
    size_t get_customer_number(size_t index) const;
private:
    void add_missing_routes(Solution &solution) const;

    void perform_relocations(std::vector<Solution> & rcl,const Solution & solution);

    template <typename Func>
    bool perform_swaps(std::vector<RCL_tuple> &rcl, const Solution &solution, Func goal) const;
    template <typename Func>
    bool perform_relocations(std::vector<RCL_tuple> &rcl, const Solution &solution, Func goal) const ;
    template <typename Func>
    bool perform_two_opt(std::vector<RCL_tuple>& rcl, const Solution& solution, Func goal) const ;
    void solve_grasp_rcl_util(std::vector<std::pair<Solution, float> > &rcl, const Solution &solution, float threshold,
                              float previous_cost) const;
    Solution get_initial_solution() const;
    bool can_add_to_route(const std::vector<size_t> &route, const DataPoint &customer)const;

private:
    std::vector<DataPoint> data;
    Graph distance_graph;
    FleetProperties fleetProperties;
    DataPoint depot;



};


#endif //OK_GRASP_PROBLEM_H
