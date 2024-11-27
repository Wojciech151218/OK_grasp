

#ifndef OK_GRASP_DATALOADER_H
#define OK_GRASP_DATALOADER_H

#include "iostream"
#include "vector"
#include "DataPoint.h"
#include <string>
#include <fstream>
#include <sstream>
#include "FleetProperties.h"
namespace DataLoader{

    std::vector<DataPoint> load_data(const std::string & file_path );
    FleetProperties load_fleet_properties(const std::string & file_path );
    DataPoint load_depot(const std::string & file_path);


}



#endif //OK_GRASP_DATALOADER_H
