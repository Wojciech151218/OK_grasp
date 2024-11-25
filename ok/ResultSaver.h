//
// Created by Wojciech on 02.11.2024.
//

#ifndef OK_GRASP_RESULTSAVER_H
#define OK_GRASP_RESULTSAVER_H

#include <string>
#include "Solution.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <iomanip>

namespace ResultSaver{
    void save_solution(Solution & solution ,Problem & problem,const std::string & result_file){
        let route_number = solution.get_routes_number();
        let distance = problem.get_cost_function(solution);

        std::ofstream file(result_file, std::ios::out);


        if(solution.is_unacceptable()){
            file<< -1;
            return;
        }

        file << route_number << " " << std::fixed << std::setprecision(5) << distance << "\n";
        // Check if the file is open
        if (!file.is_open()) {
            std::cerr << "Failed to open file." << std::endl;
            return;
        }
        for(let route : solution.getRoutes()){
            if(route.empty())continue;
            for(let vertex : route){
                file << problem.get_customer_number(vertex) << " ";
            }
            file << "\n";
        }
        file.close();
    }
};

#endif //OK_GRASP_RESULTSAVER_H
