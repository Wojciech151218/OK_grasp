

#include "DataLoader.h"

#include "utils.h"

// plik nie moze konczyc sie pustą linia inaczej nie zaladuje danych poprawnie ostatnia linijka bedzie zaladowana dwukrotnie
std::vector<DataPoint> DataLoader::load_data(const std::string & file_path ){
    std::ifstream file(file_path);
    std::vector<DataPoint> data_points;

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << file_path << std::endl;
        return data_points;  // Return an empty vector if file cannot be opened
    }

    std::string line;
    bool customer_section = false;
    auto data_lines  = false;

    // Read the file line by line
    while (std::getline(file, line)) {
        std::istringstream line_stream(line);

        // Check for section headers
        if (line.find("CUSTOMER") != std::string::npos) {
            customer_section = true;  // Begin parsing customer data after this line
            std::getline(file, line); // Skip the column headers line
            std::getline(file, line);
            continue;
        }

        if (!customer_section) continue; // Skip lines until we reach the CUSTOMER section

        if (line.find("0") != std::string::npos) {
            data_lines = true;
        }
        if(!data_lines) continue;

        if(line.empty() || line == "  "|| line == "\r") break;

        unsigned int customer_number, x, y, demand, ready, due, service;
        line_stream >> customer_number >> x >> y >> demand >> ready >> due >> service;

        // Add parsed DataPoint to the vector
        data_points.emplace_back(
                customer_number,x, y, demand, ready, due, service
        );
    }

    file.close();
    return data_points;
}

FleetProperties DataLoader::load_fleet_properties(const std::string &file_path) {
    std::ifstream file(file_path);


    if (!file.is_open()) {
        std::cerr << "Error opening file: " << file_path << std::endl;
        exit(-1);
    }

    std::string line;
    bool number_section = false;


    // Read the file line by line
    while (std::getline(file, line)) {
        std::istringstream line_stream(line);

        // Check for section headers
        if (line.find("NUMBER") != std::string::npos) {
            number_section = true;  // Begin parsing customer data after this line

            continue;
        }
        if (!number_section) continue; // Skip lines until we reach the CUSTOMER section

        if(line.empty()) break;
        // Parse customer data
        unsigned int vehicle_number,capacity;
        line_stream >> vehicle_number >> capacity;
        file.close();
        return FleetProperties(vehicle_number,capacity);
        // Add parsed DataPoint to the vector

    }
}

DataPoint DataLoader::load_depot(const std::string &file_path) {
    std::ifstream file(file_path);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << file_path << std::endl;
        exit(-1); // Return an empty vector if file cannot be opened
    }

    std::string line;
    bool customer_section = false;
    auto data_lines  = false;


    // Read the file line by line
    while (std::getline(file, line)) {
        std::istringstream line_stream(line);

        // Check for section headers
        if (line.find("CUSTOMER") != std::string::npos) {
            customer_section = true;  // Begin parsing customer data after this line
            std::getline(file, line); // Skip the column headers line
            continue;
        }
        if (!customer_section) continue;// Skip lines until we reach the CUSTOMER section
        if (line.find("0") != std::string::npos) {
            data_lines = true;
        }
        if(!data_lines) continue;
        unsigned int customer_number, x, y, demand, ready, due, service;
        line_stream >> customer_number >> x >> y >> demand >> ready >> due >> service;
        return {customer_number,x, y, demand, ready, due, service};
    }
}
