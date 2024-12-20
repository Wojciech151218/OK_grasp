
#ifndef OK_GRASP_PROBLEM_H
#define OK_GRASP_PROBLEM_H

#include <functional>
#include "vector"
#include "DataPoint.h"
#include "FleetProperties.h"
#include "Graph.h"
#include "Solution.h"
#include "utils.h"


class Problem {
private:


public:
    enum SearchResult{TimeExceeded,CriterionFulfilled,NotFulfilled};
    using CostGoalFunc = std::function<Problem::SearchResult(std::vector<RCL_tuple>&, const Solution&)>;
    void set_stop_conditions(size_t max_iterations,size_t max_time);

    Problem(std::vector<DataPoint> _data, const FleetProperties &fleetProperties, const DataPoint &depot);
    Solution solve(size_t rcl_max_size, float momentum_rate, float criterion_threshold) const;
    double get_cost_function(const Solution & solution) const;
    size_t get_customer_number(size_t index) const;
private:
    SearchResult perform_swaps(std::vector<RCL_tuple> &rcl, const Solution &solution, CostGoalFunc goal) const;
    SearchResult perform_relocations(std::vector<RCL_tuple> &rcl, const Solution &solution, CostGoalFunc goal) const ;
    SearchResult perform_two_opt(std::vector<RCL_tuple>& rcl, const Solution& solution, CostGoalFunc goal) const ;

    Solution get_initial_solution() const;
    bool can_add_to_route(const std::vector<size_t> &route, const DataPoint &customer)const;

private:
    std::vector<DataPoint> data;
    Graph distance_graph;
    FleetProperties fleetProperties;
    DataPoint depot;

    size_t max_iterations;
    size_t max_time;




};


#endif //OK_GRASP_PROBLEM_H