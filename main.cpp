#include <iostream>
#include <chrono>
#include "DataLoader.h"
#include "Problem.h"
#include "ResultSaver.h"

#include <iostream>
#include <string>
#include <cstdlib> // dla std::stoi i std::stof

int main(int argc, char* argv[]) {
    auto input_file1 = "input.txt";
    auto data = DataLoader::load_data(input_file1);
    auto fleet_properties = DataLoader::load_fleet_properties(input_file1);
    auto depot = DataLoader::load_depot(input_file1);
    auto solution = Solution::load_solution("plik-1.txt",input_file1);
    solution.is_legal(data,fleet_properties.capacity,depot);
    return 1;

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <input_file> <result_file> [max_iterations=10000] [max_time=300]\n";
        return 1;
    }

    // Pobranie argumentów
    const std::string input_file = argv[1];
    const std::string result_file = argv[2];
    size_t max_iterations = 10000;
    size_t max_time = 300;


    if (argc > 3) {
        max_iterations = std::stoi(argv[3]);
    }
    if (argc > 4) {
        max_time = std::stoi(argv[4]);
    }

    try {

        auto data = DataLoader::load_data(input_file);
        auto fleet_properties = DataLoader::load_fleet_properties(input_file);
        auto depot = DataLoader::load_depot(input_file);



        auto problem = Problem(data, fleet_properties, depot);
        problem.set_stop_conditions(max_iterations,max_time);
        auto solution = problem.solve( 10, 0.002f, 0.995);


        ResultSaver::save_solution(solution, problem, result_file);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
