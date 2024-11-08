#include <iostream>
#include "DataLoader.h"
#include "Problem.h"
#include "ResultSaver.h"

int main() {
    const std::string file_path = "../C101.txt";
    auto data = DataLoader::load_data(file_path);
    auto fleet_properties = DataLoader::load_fleet_properties(file_path);
    auto depot = DataLoader::load_depot(file_path);
    auto problem = Problem(data,fleet_properties,depot);
    auto solution = problem.solve_grasp(1000, 10,0.0f);
    ResultSaver::save_solution(solution,problem,"../result.txt");
}
