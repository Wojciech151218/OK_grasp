#include <iostream>
#include <chrono>
#include "DataLoader.h"
#include "Problem.h"
#include "ResultSaver.h"

int main() {
    const std::string file_path = "input.txt";
    // auto data1 = DataLoader::load_data(file_path);
    // auto fleet_properties1 = DataLoader::load_fleet_properties(file_path);
    // auto depot1 = DataLoader::load_depot(file_path);
    // auto problem1 = Problem(data1,fleet_properties1,depot1);
    // auto solution1 = Solution::load_solution("result.txt",file_path);
    // auto is_legal = solution1.is_legal(data1,fleet_properties1.capacity, depot1);
    // return 0;




    auto data = DataLoader::load_data(file_path);
    auto fleet_properties = DataLoader::load_fleet_properties(file_path);
    auto depot = DataLoader::load_depot(file_path);
    auto problem = Problem(data,fleet_properties,depot);
    auto start_time = std::chrono::steady_clock::now();
    auto solution = problem.solve_grasp(100, 10, 0.002f, 0.995);
    auto end_time = std::chrono::steady_clock::now();
    ResultSaver::save_solution(solution,problem,"result.txt");
    std::cout << std::endl << "Time: " << std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count() << " microseconds" << std::endl;
}
