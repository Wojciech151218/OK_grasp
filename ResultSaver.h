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

namespace ResultSaver{
    void save_solution(Solution solution ,const std::string & result_file){
        let route_number = solution.get_routes_number();
        let distance = solution.get_cost_function();

        std::ofstream file(result_file, std::ios::out);


        if(solution.is_unacceptable()){
            file<< -1;
            return;
        }

        file << route_number << " "<<distance << "\n";
        // Check if the file is open
        if (!file.is_open()) {
            std::cerr << "Failed to open file." << std::endl;
            return;
        }
        for(let route : solution.getRoutes()){
            for(let vertex : route){
                file << vertex << " ";
            }
            file << "\n";
        }
        file.close();
    }
};

#endif //OK_GRASP_RESULTSAVER_H