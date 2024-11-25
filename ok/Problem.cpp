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


Solution Problem::solve(size_t rcl_max_size, float momentum_rate, float criterion_threshold) const {

    //1 etap utworzenie pierwszego możliwego dopuszczalnego rozwiązania
    auto solution = get_initial_solution();
    if(solution.is_unacceptable())
        return solution;


    auto current_cost = INFINITY;

    size_t previous_rcl_size = 0;
    std::random_device rd;
    std::mt19937 rng(rd());
    auto start_time = std::chrono::high_resolution_clock::now();

    //funkcja kosztu po przeskalowaniu
    CostGoalFunc cost_goal = [&]
            (std::vector<RCL_tuple> &rcl, const Solution & solution) {

        let cost = get_cost_function(solution)*(solution.get_routes_number() +1);
        let momentum =criterion_threshold+ static_cast<float>( previous_rcl_size) * momentum_rate / rcl_max_size;
        let cost_after_evaluation = cost * momentum;
        if(cost_after_evaluation < current_cost)
            rcl.emplace_back(solution,cost);
        auto current_time = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count() >= max_time) {
            return Problem::SearchResult::TimeExceeded;
        }
        if(rcl.size() > rcl_max_size) return Problem::SearchResult::CriterionFulfilled;
        else return Problem::SearchResult::NotFulfilled;
    };

    // lista kandydatów
    std::vector<RCL_tuple> restricted_candidate_list = {};
    // lista kandydatów wcześniejsza lista kandydatów będzie aktualizowana za każdym razem gdy lista kandydatów nie jest pusta (dead end)
    std::vector<RCL_tuple> previous_restricted_candidate_list = {};

    using LocalSearchMethod = Problem::SearchResult (Problem::*)(std::vector<RCL_tuple>&,const Solution&, CostGoalFunc) const ;
    // metody local searchu służace do znalezenia obiecujących sąsiadów
    // jako argumenty przyjmują obecne rozwiązanie ,rozpatrywaną liczbę kandydatów
    // oraz anaonimową funkcję celu która jest zadeklarowana wyżej jako:
    // zwracają stan przeszukiwań NotFulfilled, CriterionFulfilled lub TimeExceeded

    //CostGoalFunc cost_goal = [&](std::vector<RCL_tuple> &rcl, const Solution & solution){}
    std::vector<LocalSearchMethod > local_search_methods = {
            &Problem::perform_swaps,
            &Problem::perform_relocations,
            &Problem::perform_two_opt
    };


    for (int i = 0; i < max_iterations; ++i) {

        previous_rcl_size = previous_restricted_candidate_list.size();

        auto time_constraint_met = false;
        //metody local search są tasowane
        std::shuffle(local_search_methods.begin(), local_search_methods.end(), rng);

        for (const auto& method : local_search_methods) {
            bool local_search_finished = false;
            auto result = (this->*method)(restricted_candidate_list, solution, cost_goal);
            switch (result) {
                case SearchResult::NotFulfilled :
                    //nic się nie dzieje
                    break;
                case SearchResult::CriterionFulfilled:
                    //kończy obecną iteracje
                    local_search_finished = true;
                    break;
                case SearchResult::TimeExceeded:
                    // zamyka wszelkie przeszukiwanie
                    local_search_finished = true;
                    time_constraint_met = true;
                    break;
            }
            if(local_search_finished)break;
        }
        if(time_constraint_met){
            return solution;
        }

        //gdy szukanie kandydatów się zakończymy losujemy rozwiązanie
        //z listy które zastąpi nasze poprzednie rozwiązanie w następnej iteracji
        if(!restricted_candidate_list.empty()) {
            std::uniform_int_distribution<size_t> dist(0, restricted_candidate_list.size()-1);
            let candidate_number = dist(rng);
            let& next_neighbour = restricted_candidate_list[candidate_number];
            solution = next_neighbour.first;
            current_cost = next_neighbour.second;
            remove_empty_vectors(solution.getRoutes());
            previous_restricted_candidate_list = restricted_candidate_list;
            restricted_candidate_list.clear();
        } else{
            //jeśli lista kandydatów jest pusta szukamy rozwiązań w poprzedniej liscie
            // czyli cofamyh się do stanu z poprzedniej iteracji
            if(previous_restricted_candidate_list.empty())
                return solution;
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

void Problem::set_stop_conditions(size_t max_iterations, size_t max_time) {
    this->max_iterations = max_iterations;
    this->max_time =max_time;
}

Problem::Problem(std::vector<DataPoint> _data, const FleetProperties &fleetProperties, const DataPoint &depot)
        : fleetProperties(fleetProperties) ,depot(depot) ,max_iterations(0),max_time(0){
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(_data.begin(), _data.end(), rng);
    data = _data;
    distance_graph = Graph(_data, depot);
}
/// metoda sprawdzająca możliwość dołączenia klienta do trasy (zawierającej już klientów lub pustej)
bool Problem::can_add_to_route(const std::vector<size_t> &route, const DataPoint &customer) const {
    size_t total_demand = customer.getDemand();
    auto load_time = 0.0;

    // jesli czas rozpoczęcia obsługi klienta jest większy niż czas zamkniecia depozytu nie mozna dodac do trasy  .
    if (customer.getReadyTime() > depot.getDueDate()) {
        return false;
    }


    for (size_t i = 0; i < route.size(); ++i) {
        let &next = data[route[i]];
        total_demand += next.getDemand();

        // Jeśli to nie jest pierwszy punkt na trasie, obliczamy czas załadunku od poprzedniego punktu.
        if (i > 0) {
            let &previous = data[route[i-1]];
            load_time = next.load_time(load_time, previous);

            let service = static_cast<double>(next.getService());

            // Sprawdzanie, czy przyjazd nie przekracza zamkniecia klienta
            // (w naszej implementacji load time oznacza koniec rozładunku dlatego trzeba odjąć serwis by otrzymać przyjazd do klienta)
            if (load_time - service > next.getDueDate()) {
                return false;
            }
        } else {
            // Dla pierwszego punktu trasy, czas załadunku liczymy od depozytu.
            load_time =  next.load_time(load_time, depot);
        }
    }


    // jesli trasa jest pusta liczymy odległość dla rozpatrywanego kleinta do depotu
    // w przeciwnym razie liczymy odległość od ostatniego klienta na trasie
    load_time = route.empty() ? customer.load_time(load_time, depot)
            : customer.load_time(load_time, data[route[route.size() - 1]]);

    //dodajemy powrót do depota
    let load_at_depot = depot.load_time(load_time, customer);

    // Sprawdzanie, czy czas powrotu do depotu przekracza zamkniecie depotu (powrót nie byłby możliwy).
    if (load_at_depot > static_cast<double>(depot.getDueDate())) {
        return false; // Jeśli tak, trasa nie jest możliwa.
    }

    // Sprawdzanie, czy całkowite zapotrzebowanie przekracza pojemność cieżarówki
    // oraz czy czas załadunku jest zgodny z ograniczeniami czasowymi klienta.
    return total_demand <= fleetProperties.capacity &&
           load_time - customer.getService() <= static_cast<double>(customer.getDueDate());
}


///tworzy początkowe rozwiązanie
Solution Problem::get_initial_solution() const {
    // utworzenie rozwiaząnia bez utworzonych tras
    // wierczholki sa potasowane
    Solution initial_solution(distance_graph);

    for (size_t index = 0 ; index< data.size();index++) {
        bool placed = false;
        let customer = data[index];
        for (auto &route : initial_solution.getRoutes()) {
            //weryfikujemy czy wierzcholek moze byc dodany do trasy
            //jesli nie szukamy nastepnej trasy i sprawdzamy jeszcze raz
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
/// właściwa funkcja celu czyli długość tras + czekanie + serwis
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






