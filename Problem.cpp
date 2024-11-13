//
// Created by Wojciech on 01.11.2024.
//

#include "Problem.h"
#include "Solution.h"
#include "utils.h"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <random>
#include <vector>
#include <functional>
#include <iomanip>
#include <chrono>


Solution Problem::solve_grasp(size_t epochs, size_t rcl_max_size, float momentum_rate, float criterion_threshold) const {
    auto time_limit = 300;
    auto solution = get_initial_solution();
    auto current_cost = INFINITY;
    size_t previous_rcl_size = 0;
    std::random_device rd;
    std::mt19937 rng(rd());
    auto start_time = std::chrono::high_resolution_clock::now();

    CostGoalFunc cost_goal = [&]
            (std::vector<RCL_tuple> &rcl, const Solution & solution) {
        let excessive_routes =  solution.get_routes_number()>fleetProperties.vehicle_number?
                                solution.get_routes_number()-fleetProperties.vehicle_number - count_empty_vectors(solution.getRoutes()):0;
        let cost = get_cost_function(solution)*(excessive_routes +1);
        let momentum =criterion_threshold+ static_cast<float>( previous_rcl_size) * momentum_rate / rcl_max_size;
        let cost_after_evaluation = cost * momentum;
        if(cost_after_evaluation < current_cost)
            rcl.emplace_back(solution,cost);
        auto current_time = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count() >= time_limit) {
            return Problem::SearchResult::TimeExceeded;
        }
        if(rcl.size() > rcl_max_size) return Problem::SearchResult::CriterionFulfilled;
        else return Problem::SearchResult::NotFulfilled;
    };


    std::vector<RCL_tuple> restricted_candidate_list = {};
    std::vector<RCL_tuple> previous_restricted_candidate_list = {};

    using LocalSearchMethod = Problem::SearchResult (Problem::*)(std::vector<RCL_tuple>&,const Solution&, CostGoalFunc) const ;

    std::vector<LocalSearchMethod > local_search_methods = {
            &Problem::perform_swaps,
            &Problem::perform_relocations,
            &Problem::perform_two_opt
    };


    for (int i = 0; i < epochs; ++i) {
        if(i%1==0){
            std::cout<< i ;
            std::cout<<std::fixed << std::setprecision(5) << " cost " << current_cost << " size "<<solution.get_routes_number() <<"\n";
        }
        previous_rcl_size = previous_restricted_candidate_list.size();

        auto time_constraint_met = false;
        std::shuffle(local_search_methods.begin(), local_search_methods.end(), rng);
        for (const auto& method : local_search_methods) {
            bool local_search_finished = false;
            auto result = (this->*method)(restricted_candidate_list, solution, cost_goal);
            switch (result) {
                case SearchResult::NotFulfilled : break;
                case SearchResult::CriterionFulfilled:
                    local_search_finished = true;
                    break;
                case SearchResult::TimeExceeded:
                    local_search_finished = true;
                    time_constraint_met = true;
                    break;
            }
            if(local_search_finished)break;
        }
        if(time_constraint_met){
            auto current_time = std::chrono::high_resolution_clock::now();
            std::cerr<<"time constraint met "<< std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time).count();
            break;
        }


        if(! restricted_candidate_list.empty()) {
            std::uniform_int_distribution<size_t> dist(0, restricted_candidate_list.size()-1);
            let candidate_number = dist(rng);
            let& next_neighbour = restricted_candidate_list[candidate_number];
            solution = next_neighbour.first;
            current_cost = next_neighbour.second;
            remove_empty_vectors(solution.getRoutes());
            previous_restricted_candidate_list = restricted_candidate_list;
            restricted_candidate_list.clear();
        } else{
            std::uniform_int_distribution<size_t> dist(0, previous_restricted_candidate_list.size()-1);
            let candidate_number = dist(rng);
            let& next_neighbour = previous_restricted_candidate_list[candidate_number];
            solution = next_neighbour.first;
            current_cost = next_neighbour.second;
            remove_empty_vectors(solution.getRoutes());
        }
    }
    return solution;
}

Problem::Problem(std::vector<DataPoint> _data, const FleetProperties &fleetProperties, const DataPoint &depot)
        : fleetProperties(fleetProperties) ,depot(depot) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(_data.begin(), _data.end(), rng);
    data = _data;
    distance_graph = Graph(_data, depot);
}
bool Problem::can_add_to_route(const std::vector<size_t> &route, const DataPoint &customer)const {
    size_t total_demand = customer.getDemand();
;
    auto load_time = 0.0f;
    if(customer.getReadyTime() > depot.getDueDate()){
        return false;
    }

    for (size_t i = 0; i < route.size(); ++i) {
        let &next = data[route[i]];
        total_demand += next.getDemand();


        if (i > 0) {
            let &previous = data[route[i-1]] ;

            load_time = next.load_time(load_time, previous);
            let service = static_cast<float>(next.getService());
            if (load_time - service > static_cast<float>(next.getDueDate())) return false;
            // Time window constraint violated

        }else {
            load_time = static_cast<float>(next.getReadyTime());
        }
    }
    load_time = route.empty() ? static_cast<float>(depot.getReadyTime()) : customer.load_time(load_time,data[route[route.size()-1]]);
    if(depot.load_time(load_time,customer)> static_cast<float>(depot.getDueDate())) {
        return false; // cant go back nie mozemy tu isc
    }

    return total_demand <= fleetProperties.capacity && load_time <= static_cast<float>(customer.getDueDate());
}


Solution Problem::get_initial_solution() const {
    Solution initial_solution(distance_graph);

    for (size_t index = 0 ; index< data.size();index++) {
        bool placed = false;
        let customer = data[index];
        for (auto &route : initial_solution.getRoutes()) {
            if (can_add_to_route(route, customer)) {
                route.push_back(index);
                placed = true;
                break;
            }
        }
        if (!placed) {
            std::vector<size_t> new_route = {};
            if (can_add_to_route(new_route,customer) ){
                new_route.push_back(index);
                initial_solution.getRoutes().push_back(new_route);
            } else {
                    std::cerr<< "unable to create an acceptable initial solution.\n";
                initial_solution.setUnacceptable();
                return initial_solution;
            }
        }
    }
    return initial_solution;

}

double Problem::get_cost_function(const Solution &solution) const {
   auto result = 0.0;

   for(let &route: solution.getRoutes()){
       if(route.empty()) continue;
       auto load_time = 0.0;
       auto previous_node = depot;
       for(let node : route){
           load_time = data[node].load_time(load_time, previous_node);
           previous_node = data[node];
       }
       result += load_time + depot.get_distance(previous_node);
   }

   return result;
}

size_t Problem::get_customer_number(size_t index) const {
    return data[index].getCustomerNumber();
}

void Problem::add_missing_routes(Solution &solution) const {
    int missing_routes = static_cast<int>(fleetProperties.vehicle_number) - solution.get_routes_number();
    if(missing_routes > 0) {
        for (auto i =0;i<missing_routes;i++) {
            solution.getRoutes().emplace_back();
        }
    }
}


Problem::SearchResult Problem::perform_swaps(std::vector<RCL_tuple>& rcl, const Solution& solution, CostGoalFunc goal) const {
    const auto& routes = solution.getRoutes();
    size_t route_count = routes.size();

    // Generate a shuffled order of route indices
    auto shuffled_routes = getShuffledIndices(route_count);

    // Loop through each route in random order
    for (size_t route_number : shuffled_routes) {
        const auto& route = routes[route_number];
        size_t route_size = route.size();

        // Generate a shuffled order of node indices within the route
        auto shuffled_nodes = getShuffledIndices(route_size);

        // Loop through each pair of nodes by shuffled index within the route
        for (size_t first_index : shuffled_nodes) {
            for (size_t second_index : shuffled_nodes) {
                if (first_index == second_index) continue; // Skip self-swap

                // Create a candidate solution by swapping nodes at shuffled indices
                auto candidate_solution = solution.swap(route_number, first_index, second_index);

                // Check legality and evaluate the goal
                if (candidate_solution.is_legal(data, fleetProperties.capacity, depot)) {
                    let goal_result =goal(rcl, candidate_solution);
                    if (goal_result != Problem::SearchResult::NotFulfilled) return goal_result;
                }
            }
        }
    }

    return Problem::SearchResult::NotFulfilled; // Return false if no satisfactory solution was found
}


Problem::SearchResult Problem::perform_relocations(std::vector<RCL_tuple>& rcl, const Solution& solution, CostGoalFunc goal) const {
    auto routes = solution.getRoutes();
    size_t route_count = routes.size();

    // Generate a shuffled order of source route indices
    auto shuffled_source_routes = getShuffledIndices(route_count);

    // Loop over each source route in random order
    for (size_t source_route_index : shuffled_source_routes) {
        const auto& source_route = routes[source_route_index];
        size_t source_size = source_route.size();

        // Generate a shuffled order of node indices within the source route
        auto shuffled_nodes = getShuffledIndices(source_size);

        // Loop through each node by shuffled index in the source route
        for (size_t node_index : shuffled_nodes) {
            // Generate a shuffled order of target route indices
            auto shuffled_target_routes = getShuffledIndices(route_count);

            // Try relocating this node to each target route in random order
            for (size_t target_route_index : shuffled_target_routes) {
                if (source_route_index == target_route_index) continue; // Skip self-relocation

                const auto& target_route = routes[target_route_index];
                size_t target_size = target_route.size();

                // Generate a shuffled order of insertion positions in the target route
                auto shuffled_positions = getShuffledIndices(target_size + 1);

                // Try each position in the target route
                for (size_t target_position : shuffled_positions) {
                    // Create a candidate solution with relocation
                    auto candidate_solution = solution.relocation(source_route_index, target_route_index, node_index, target_position);

                    // Check legality and evaluate the goal
                    if (candidate_solution.is_legal(data, fleetProperties.capacity, depot)) {
                        let goal_result =goal(rcl, candidate_solution);
                        if (goal_result != Problem::SearchResult::NotFulfilled) return goal_result;
                    }
                }
            }
        }
    }

    return Problem::SearchResult::NotFulfilled;
}

Problem::SearchResult Problem::perform_two_opt(std::vector<RCL_tuple>& rcl, const Solution& solution, CostGoalFunc goal) const {
    auto routes = solution.getRoutes();
    size_t route_count = routes.size();

    // Generate a shuffled order of route indices
    auto shuffled_routes = getShuffledIndices(route_count);

    // Loop over each route in random order
    for (size_t route_index : shuffled_routes) {
        const auto& route = routes[route_index];
        size_t route_size = route.size();

        if (route_size < 2) continue; // Skip routes with fewer than 2 nodes

        // Generate shuffled indices for two-opt segments within the route
        auto shuffled_start_indices = getShuffledIndices(route_size - 1);

        // Loop over random start and end indices for two-opt within the route
        for (size_t start_index : shuffled_start_indices) {
            auto shuffled_end_indices = getShuffledIndices(route_size - start_index - 1);

            for (size_t end_offset : shuffled_end_indices) {
                size_t end_index = start_index + 1 + end_offset;

                // Create a candidate solution by applying two-opt
                auto candidate_solution = solution.two_opt(route_index, start_index, end_index);

                // Check legality and evaluate the goal
                if (candidate_solution.is_legal(data, fleetProperties.capacity, depot)) {
                    let goal_result =goal(rcl, candidate_solution);
                    if (goal_result != Problem::SearchResult::NotFulfilled) return goal_result;
                }
            }
        }
    }

    return Problem::SearchResult::NotFulfilled;
}






