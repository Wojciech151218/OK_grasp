//
// Created by Wojciech on 02.11.2024.
//

#include <ctime>
#include "Solution.h"
#include "utils.h"
#include "DataLoader.h"
#include <vector>
#include <random>
#include <algorithm>
#include <stdexcept>

typedef std::mt19937 rng_type;
rng_type rng(std::random_device{}()); // Initialize the random number generator


Solution::Solution(const Graph  & graph): graph(graph), unacceptable(false){}




Solution Solution::relocation(size_t route, size_t target_route, size_t index, size_t target_index) const {
    //zrobione przez chat gpt
    Solution new_solution = *this;  // Create a copy of the current solution

    // Check if the specified route and index are within bounds
    if (route >= new_solution.routes.size() || index >= new_solution.routes[route].size() ||
        target_route >= new_solution.routes.size()) {
        throw std::out_of_range("Invalid route or index in relocation operation");
        }

    // Get a reference to the source and target routes
    auto& source_route = new_solution.routes[route];
    auto& destination_route = new_solution.routes[target_route];

    // Extract the node to relocate
    auto node_to_relocate = source_route[index];

    // Remove the node from the source route
    source_route.erase(source_route.begin() + index);

    // Insert the node into the target route at the specified position
    if (target_index > destination_route.size()) {
        target_index = destination_route.size(); // Insert at end if target_index is out of bounds
    }
    destination_route.insert(destination_route.begin() + target_index, node_to_relocate);

    // Return the modified solution with the relocated node
    return new_solution;
}


Solution Solution::two_opt(size_t route_index, size_t start_index, size_t end_index) const{
    //zrobione przez chat gpt
    Solution new_solution = *this;  // Copy the current solution

    // Check if the specified route and indices are within bounds
    if (route_index >= new_solution.routes.size() ||
        start_index >= new_solution.routes[route_index].size() ||
        end_index >= new_solution.routes[route_index].size() ||
        start_index >= end_index) {
        throw std::out_of_range("Invalid route or index in two_opt operation");
        }

    // Get a reference to the specific route to modify
    auto& route = new_solution.routes[route_index];

    // Reverse the segment between start_index and end_index (inclusive)
    std::reverse(route.begin() + start_index, route.begin() + end_index + 1);

    // Return the modified solution with the updated route
    return new_solution;
}




Solution Solution::swap(size_t route_number, size_t node_a, size_t node_b) const{
    Solution new_solution = *this;
    std::swap(new_solution.routes[route_number][node_a] ,new_solution.routes[route_number][node_b]);

    return new_solution;
}
bool Solution::is_legal(const std::vector<DataPoint> & data, size_t capacity, const DataPoint &depot) const {

    for (let &route : routes) {
        size_t total_demand = 0;
        double load_time = depot.getReadyTime();

        for (size_t i = 0; i < route.size(); ++i) {
            let &current_vertex = data[route[i]];
            total_demand += current_vertex.getDemand();
            if (total_demand > capacity) {
                return false;  // Przekroczono pojemność
            }

            // czas załadunku dla danego wierzchołka
            if (i > 0) {
                const DataPoint &previous_vertex = data[route[i-1]];
                load_time = current_vertex.load_time(load_time, previous_vertex);
            } else {
                // Pierwszy przystanek po depocie, ustaw czas załadunku na najwcześniejszy czas gotowości
                load_time = current_vertex.load_time(load_time, depot);
            }

            // Sprawdź, czy czas przyjazdu mieści się w dopuszczalnym przedziale czasowym
            if (load_time >  static_cast<double>(current_vertex.getDueDate()+current_vertex.getService())) {
                return false;  // Naruszono ograniczenie okna czasowego
            }
            if(i == route.size() -1) {
                let load_at_depot = depot.load_time(load_time,current_vertex);
                if(load_at_depot > static_cast<double>(depot.getDueDate()))
                    return false;

            }
        }


    }
    return true;  // Wszystkie trasy spełniają ograniczenia pojemności i okna czasowego
}


std::vector<std::vector<size_t>> &Solution::getRoutes()  {
    return routes;

}

size_t Solution::get_routes_number() const {
    return routes.size();
}

bool Solution::is_unacceptable() const {
    return unacceptable;
}

void Solution::setUnacceptable() {
    unacceptable = true;
}




std::vector<std::vector<size_t>> Solution::getRoutes() const {
    return routes;
}

Solution Solution::load_solution(const std::string &result_file, const std::string &input_file) {
    auto data = DataLoader::load_data(input_file);
    auto depot = DataLoader::load_depot(input_file);
    auto graph = Graph(data, depot);
    auto solution = Solution(graph);

    std::ifstream file(result_file);

    if (!file.is_open()){
        std::cerr << "Unable to open file!" << std::endl;
        return solution;
    }

    std::string line;
    std::vector<std::vector<size_t>> routes;
    std::getline(file, line);
    while (std::getline(file, line)) {
        std::vector<size_t> route;


        std::istringstream line_stream(line);
        size_t customer_number;
        while (line_stream >> customer_number) {
            route.push_back(customer_number-1);
        }
        routes.push_back(route);
    }


    solution.routes = routes;
    file.close();

    return solution;
}


