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



Solution Problem::solve_grasp(size_t epochs, float randomness_level) const  {

    let solution = get_initial_solution();
    for (int i = 0; i < epochs; ++i) {
        //todo
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
    size_t total_demand = 0;
    auto load_time = 0.0f;


    for (size_t i = 0; i < route.size(); ++i) {
        let &next = data[route[i]];
        total_demand += next.getDemand();

        if(next.getReadyTime() > depot.getDueDate()){
            return false;
        }
        if (i > 0) {
            let &previous = data[route[i-1]] ;

            load_time = next.load_time(load_time, previous);
            let service = static_cast<float>(next.getService());
            if (load_time - service > static_cast<float>(next.getDueDate())) return false;
            if(depot.load_time(load_time,previous)> static_cast<float>(depot.getDueDate())){
                return false; // cant go back nie mozemy tu isc
            }// Time window constraint violated

        }else {
            load_time = static_cast<float>(next.getReadyTime());
        }
    }

    // Check capacity and time for the new customer
    total_demand += customer.getDemand();
    load_time = customer.load_time(load_time,data[route[route.size()-1]]);

    return total_demand <= fleetProperties.capacity && load_time <= static_cast<float>(customer.getDueDate());
}
bool Problem::check_capacity(const std::vector<size_t> &route, const DataPoint &customer) const {
    size_t total_demand = 0;
    for (let &i : route) {
        total_demand += data[i].getDemand();
    }
    return total_demand <= fleetProperties.capacity;
}
bool Problem::check_time_window(const std::vector<size_t> &route, const DataPoint &customer) const {
    auto load_time = 0.0f;

    for (size_t i = 0; i < route.size(); ++i) {
        let &next = data[route[i]];
        if(next.getReadyTime() > depot.getDueDate()){
            return false;
        }

        if (i > 0) {
            let &previous = data[route[i-1]];
            load_time = next.load_time(load_time, previous);
            let service = static_cast<float>(next.getService());
            if (load_time - service> static_cast<float>(next.getDueDate())) {
                return false;  // Time window constraint violated
            }
            if(depot.load_time(load_time,previous)> static_cast<float>(depot.getDueDate())){
                return false; // cant go back nie mozemy tu isc
            }
        } else {
            load_time = static_cast<float>(next.getReadyTime());
        }
    }
    return true;
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
            std::vector<size_t> new_route = {index};
            if (check_capacity(new_route, customer) &&
                check_time_window(new_route, customer)) {
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
       result += static_cast<float>(load_time) + depot.get_distance(previous_node);
   }

   return result;
}

size_t Problem::get_customer_number(size_t index) const {
    return data[index].getCustomerNumber();
}


