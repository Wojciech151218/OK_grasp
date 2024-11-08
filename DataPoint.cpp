//
// Created by Wojciech on 01.11.2024.
//

#include "DataPoint.h"
#include "cmath"
#include "utils.h"

DataPoint::DataPoint(unsigned int customer,unsigned int x, unsigned int y, unsigned int demand,
                     unsigned int ready, unsigned int due, unsigned int service)
        :customer_number(customer), x_coordinate(x), y_coordinate(y), demand(demand),
        ready_time(ready),due_date(due), service(service) {}

float DataPoint::get_distance(const DataPoint &dataPoint) const {
    double dx = static_cast<double>(dataPoint.x_coordinate) - static_cast<double>(x_coordinate);
    double dy = static_cast<double>(dataPoint.y_coordinate) - static_cast<double>(y_coordinate);

    return hypot(dx,dy);
}


unsigned int DataPoint::getService() const {
    return service;
}

unsigned int DataPoint::getDueDate() const {
    return due_date;
}

unsigned int DataPoint::getReadyTime() const {
    return ready_time;
}

unsigned int DataPoint::getDemand() const {
    return demand;
}

float DataPoint::load_time(float previous_load_time, const DataPoint & previous_vertex) const {
    /// b_i = max{ b_i-1 + d_i-1 + c_i-1,i , e_i  }
    /// b_1-1 czas zakończenia rozładunku w poprzednim wierzchołku.
    ///d_i-1 czas rozładunku w poprzednim wierzchołku.
    ///c_i-1,i czas przejazdu między wierzchołkami i-1 i i
    ///e_i  najwcześniejszy dopuszczalny czas rozładunku w wierzchołku i

    let next_available_time = static_cast<float>(ready_time);


    let finish_time = previous_load_time;
    let previous_reload_time = static_cast<float>( previous_vertex.getService());
    let distance =  get_distance(previous_vertex) ;


    return std::max(finish_time + previous_reload_time + distance + static_cast<float>(service) ,
        next_available_time+static_cast<float>(service));
}

unsigned int DataPoint::getCustomerNumber() const {
    return customer_number;
}


