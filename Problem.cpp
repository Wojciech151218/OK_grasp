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



Solution Problem::solve_grasp(size_t epochs, float randomness_level) const {
    auto solution = get_initial_solution();
    auto current_cost = get_cost_function(solution);

    solution.fit_to_constraints(fleetProperties.vehicle_number);

    for (int i = 0; i < epochs; ++i) {
        let routes_size = solution.get_routes_number();

        for (auto first_route_index = 0 ; first_route_index < routes_size ; first_route_index++  ) {
            for (let & first_node : solution.getRoutes()[first_route_index]) {
                for (let & second_node : solution.getRoutes()[first_route_index]) {

                    if(first_node == second_node) continue;

                    for(auto second_route_index = 0 ; second_route_index < routes_size ; second_route_index++ ) {

                        if(first_route_index == second_route_index) continue;
                        let candidate_solution = solution.relocation(first_route_index,second_route_index,
                            first_route_index,second_route_index);

                        if(solution.is_legal(data,fleetProperties.capacity)) {
                            let cost = get_cost_function(candidate_solution);
                            if(cost < current_cost ) {
                                current_cost = cost;
                                solution = candidate_solution;
                                goto next_epoch;
                            }
                        }
                    }

                    auto candidate_solution = solution.swap(first_route_index,first_node,second_node);
                    if(solution.is_legal(data,fleetProperties.capacity)) {
                        let cost = get_cost_function(candidate_solution);
                        if(cost < current_cost ) {
                            current_cost = cost;
                            solution = candidate_solution;
                            goto next_epoch;
                        }
                    }

                    // candidate_solution = solution.two_opt(first_route_index,first_node,second_node);
                    // if(solution.is_legal(data,fleetProperties.capacity)) {
                    //     let cost = get_cost_function(candidate_solution);
                    //     if(cost < current_cost ) {
                    //         current_cost = cost;
                    //         solution = candidate_solution;
                    //         goto next_epoch;
                    //     }
                    // }
                }
            }
        }
        next_epoch:;
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


