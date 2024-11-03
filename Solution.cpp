//
// Created by Wojciech on 02.11.2024.
//

#include <ctime>
#include "Solution.h"
#include "utils.h"
#include <vector>
#include <random>
#include <algorithm>

typedef std::mt19937 rng_type;
rng_type rng(std::random_device{}()); // Initialize the random number generator


Solution::Solution(const Graph  & graph): graph(graph), unacceptable(false){}




Solution Solution::relocation() {
    Solution new_solution = *this;

    // Select a source vehicle route with at least 2 nodes
    size_t source_vehicle;
    std::uniform_int_distribution<size_t> vehicle_dist(0, routes.size() - 1);
    do {
        source_vehicle = vehicle_dist(rng);
    } while (new_solution.routes[source_vehicle].size() < 2);

    // Select the node to relocate
    auto &source_route = new_solution.routes[source_vehicle];
    std::uniform_int_distribution<size_t> node_dist(0, source_route.size() - 1);
    size_t node_index = node_dist(rng);
    size_t node_to_move = source_route[node_index];

    // Remove the node from the source route
    source_route.erase(source_route.begin() + node_index);

    // Select a destination vehicle (could be the same as source)
    size_t destination_vehicle = vehicle_dist(rng);
    auto &destination_route = new_solution.routes[destination_vehicle];

    // Select insertion position within the destination route
    std::uniform_int_distribution<size_t> insert_dist(0, destination_route.size());
    size_t insert_position = insert_dist(rng);

    // Insert the node at the new position
    destination_route.insert(destination_route.begin() + insert_position, node_to_move);

    return new_solution;
}


Solution Solution::three_opt() {
    Solution new_solution = *this;

    // Select a vehicle route with at least 4 nodes
    size_t vehicle_index;
    std::uniform_int_distribution<size_t> vehicle_dist(0, routes.size() - 1);
    do {
        vehicle_index = vehicle_dist(rng);
    } while (new_solution.routes[vehicle_index].size() < 4);

    auto &route = new_solution.routes[vehicle_index];
    std::uniform_int_distribution<size_t> node_dist(0, route.size() - 1);

    // Select three breakpoints a, b, and c, where a < b < c
    size_t a = node_dist(rng) % (route.size() - 3);
    size_t b = a + 1 + (node_dist(rng) % (route.size() - a - 2));
    size_t c = b + 1 + (node_dist(rng) % (route.size() - b - 1));

    // Split the route into three segments and rearrange
    std::vector<size_t> segment1(route.begin(), route.begin() + a + 1);
    std::vector<size_t> segment2(route.begin() + a + 1, route.begin() + b + 1);
    std::vector<size_t> segment3(route.begin() + b + 1, route.begin() + c + 1);

    // Concatenate segments in a different order
    route = segment1;
    route.insert(route.end(), segment3.begin(), segment3.end());
    route.insert(route.end(), segment2.begin(), segment2.end());

    return new_solution;
}


Solution Solution::two_opt() {
    Solution new_solution = *this;

    // Select a vehicle route with at least 3 nodes
    size_t vehicle_index;
    std::uniform_int_distribution<size_t> vehicle_dist(0, routes.size() - 1);
    do {
        vehicle_index = vehicle_dist(rng);
    } while (new_solution.routes[vehicle_index].size() < 3);

    auto &route = new_solution.routes[vehicle_index];
    std::uniform_int_distribution<size_t> node_dist(0, route.size() - 1);

    // Select two indices i and j where i < j
    size_t i = node_dist(rng);
    size_t j;
    do {
        j = node_dist(rng);
    } while (i >= j);  // Ensure that i < j

    // Reverse the section between i and j
    std::reverse(route.begin() + i, route.begin() + j + 1);

    return new_solution;
}



Solution Solution::swap() {
    Solution new_solution = *this;

    // Randomly select a vehicle route with at least 2 nodes
    size_t vehicle_index;
    std::uniform_int_distribution<size_t> vehicle_dist(0, routes.size() - 1);
    do {
        vehicle_index = vehicle_dist(rng);
    } while (new_solution.routes[vehicle_index].size() < 2);

    // Get the route of the selected vehicle
    auto &route = new_solution.routes[vehicle_index];
    std::uniform_int_distribution<size_t> node_dist(0, route.size() - 1);

    // Select two distinct indices for swapping
    size_t i = node_dist(rng);
    size_t j;
    do {
        j = node_dist(rng);
    } while (i == j);  // Ensure that i and j are different

    // Perform the swap
    std::swap(route[i], route[j]);

    return new_solution;
}
bool Solution::is_legal(const std::vector<DataPoint> & data, size_t capacity) const {

    for (let &route : routes) {
        size_t total_demand = 0;  // Sum of demands in the current route
        size_t load_time = 0;     // Initial load time (start from the depot)

        for (size_t i = 0; i < route.size(); ++i) {
            let &current_vertex = data[route[i]];

            // Accumulate demand for capacity check
            total_demand += current_vertex.getDemand();
            if (total_demand > capacity) {
                return false;  // Capacity exceeded
            }

            // Calculate the load time for this stop
            if (i > 0) {
                const DataPoint &previous_vertex = data[route[i-1]];
                load_time = current_vertex.load_time(load_time, previous_vertex);
            } else {
                // First stop after the depot, set load time to earliest ready time if starting from the depot
                load_time = current_vertex.getReadyTime();
            }

            // Check if arrival time respects the due time
            if (load_time > current_vertex.getDueDate()) {
                return false;  // Time window constraint violated
            }
        }
    }
    return true;  // All routes satisfy the capacity and time window constraints
}

std::vector<std::vector<size_t>> &Solution::getRoutes()  {
    return routes;

}

float Solution::get_routes_number() const {
    return routes.size();
}

bool Solution::is_unacceptable() const {
    return unacceptable;
}

void Solution::setUnacceptable() {
    Solution::unacceptable = true;
}

std::vector<std::vector<size_t>> Solution::getRoutes() const {
    return routes;
}

