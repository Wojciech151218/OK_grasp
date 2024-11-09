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

void Problem::solve_grasp_rcl_util(std::vector<RCL_tuple> &rcl, const Solution & solution, float threshold, float previous_cost) const {
    let cost = get_cost_function(solution);
    if(cost > previous_cost*threshold) {
        rcl.emplace_back(solution,cost);
    }

}

Solution Problem::solve_grasp(size_t epochs, size_t rcl_size, float threshold) const {
    auto solution = get_initial_solution();
    add_missing_routes(solution);
    auto current_cost = get_cost_function(solution);
    std::random_device rd;
    std::mt19937 rng(rd());

    //remove_empty_vectors(solution.getRoutes());

    auto cost_goal = [threshold, current_cost,rcl_size, this](std::vector<RCL_tuple> &rcl,const Solution & solution) {
        let cost = get_cost_function(solution);
        if(cost < current_cost*threshold)
            rcl.emplace_back(solution,cost);
        return rcl.size() > rcl_size/3;
    };
    auto route_number_goal = [this, rcl_size](std::vector<RCL_tuple> &rcl,const Solution & solution) {
        let empty_routes = count_empty_vectors(solution.getRoutes());
        static std::random_device rd;
        static std::mt19937 rng(rd());
        std::uniform_real_distribution<float> dist(0.0, rcl_size);
        auto probability = rcl.size()/rcl_size;
        if(empty_routes > 0 or dist(rng) < probability)
            rcl.emplace_back(solution,0.0);
        return rcl.size() > rcl_size;

    };

    std::function<bool(std::vector<RCL_tuple>&, const Solution&)> lambda;

    for (int i = 0; i < epochs; ++i) {
        if(solution.get_routes_number()<= fleetProperties.vehicle_number)
            lambda = cost_goal;
        else
            lambda = route_number_goal;

        std::vector<RCL_tuple> restricted_candidate_list = {};

        if(perform_swaps(restricted_candidate_list,solution,lambda)){}

        if(perform_relocations(restricted_candidate_list,solution,lambda)){}

        if(perform_two_opt(restricted_candidate_list,solution,lambda)){}


        if(not restricted_candidate_list.empty()) {
            std::uniform_int_distribution<size_t> dist(0, restricted_candidate_list.size()-1);
            let candidate_number = dist(rng);
            let& next_neighbour = restricted_candidate_list[candidate_number];
            solution = next_neighbour.first;
            current_cost = next_neighbour.second;
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

float Problem::get_cost_function(const Solution &solution) const {
   auto result = 0.0f;

   for(let &route: solution.getRoutes()){
       auto load_time = 0.0f;
       auto previous_node = depot;
       for(let node : route){
           load_time += data[node].load_time(load_time, previous_node);
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


template <typename Func>
bool Problem::perform_swaps(std::vector<RCL_tuple>& rcl, const Solution& solution, Func goal) const {
    const auto& routes = solution.getRoutes();  // Access the routes directly once
    size_t route_count = routes.size();

    // Loop through each route by index
    for (size_t route_number = 0; route_number < route_count; ++route_number) {
        const auto& route = routes[route_number];
        size_t route_size = route.size();

        // Loop through each pair of nodes by index within the route
        for (size_t first_index = 0; first_index < route_size; ++first_index) {
            for (size_t second_index = 0; second_index < route_size; ++second_index) {

                // Skip if the indices are the same, to avoid self-swap
                if (first_index == second_index) continue;

                // Create a candidate solution by swapping the nodes at first_index and second_index
                auto candidate_solution = solution.swap(route_number, first_index, second_index);

                // Check if the candidate solution is legal
                if (candidate_solution.is_legal(data, fleetProperties.capacity)) {
                    // Evaluate the candidate solution with the goal function
                    if (goal(rcl, candidate_solution)) return true;  // Stop if the goal is satisfied
                }
            }
        }
    }

    return false;  // Return false if no satisfactory solution was found
}

template <typename Func>
bool Problem::perform_relocations(std::vector<RCL_tuple> &rcl, const Solution &solution, Func goal) const {
    auto routes = solution.getRoutes();
    size_t route_count = routes.size();

    // Loop over each route as a source route for relocation
    for (size_t source_route_index = 0; source_route_index < route_count; ++source_route_index) {
        const auto& source_route = routes[source_route_index];

        // Loop through each node in the source route
        for (size_t node_index = 0; node_index < source_route.size(); ++node_index) {

            // Try relocating this node into each target route
            for (size_t target_route_index = 0; target_route_index < route_count; ++target_route_index) {

                // Skip relocation within the same route
                if (source_route_index == target_route_index) continue;

                const auto& target_route = routes[target_route_index];

                // Attempt relocation to every position in the target route
                for (size_t target_position = 0; target_position <= target_route.size(); ++target_position) {
                    // Create a candidate solution with relocation
                    auto candidate_solution = solution.relocation(source_route_index, target_route_index, node_index, target_position);

                    // Check if the candidate solution is legal (meets constraints)
                    if (candidate_solution.is_legal(data, fleetProperties.capacity)) {
                        // Evaluate candidate solution with the goal function
                        if(goal(rcl, candidate_solution)) return true;
                    }
                }
            }
        }
    }
    return false;
}
template <typename Func>
bool Problem::perform_two_opt(std::vector<RCL_tuple>& rcl, const Solution& solution, Func goal) const {
    auto routes = solution.getRoutes();
    size_t route_count = routes.size();

    // Loop over each route to apply two-opt within each route
    for (size_t route_index = 0; route_index < route_count; ++route_index) {
        const auto& route = routes[route_index];
        size_t route_size = route.size();

        // Loop through each possible segment within the route
        for (size_t start_index = 0; start_index < route_size - 1; ++start_index) {
            for (size_t end_index = start_index + 1; end_index < route_size; ++end_index) {

                // Create a candidate solution by applying two-opt to the segment
                auto candidate_solution = solution.two_opt(route_index, start_index, end_index);

                // Check if the candidate solution is legal (meets constraints)
                if (candidate_solution.is_legal(data, fleetProperties.capacity)) {
                    // Evaluate candidate solution with the goal function
                    if (goal(rcl, candidate_solution)) return true;  // Stop if goal is satisfied
                }
            }
        }
    }
    return false;  // Return false if no satisfactory solution was found
}





