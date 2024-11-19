#include <iostream>
#include <chrono>
#include "DataLoader.h"
#include "Problem.h"
#include "ResultSaver.h"

int main() {
    const std::string file_path = "input.txt";

    auto data = DataLoader::load_data(file_path);
    auto fleet_properties = DataLoader::load_fleet_properties(file_path);
    auto depot = DataLoader::load_depot(file_path);
    auto problem = Problem(data,fleet_properties,depot);
    auto start_time = std::chrono::steady_clock::now();
    auto solution = problem.solve_grasp(10000, 5, 0.003f, 0.995);
    auto end_time = std::chrono::steady_clock::now();
    ResultSaver::save_solution(solution,problem,"result.txt");
    std::cout << std::endl << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() << " microseconds" << std::endl;

    return 0;
}
