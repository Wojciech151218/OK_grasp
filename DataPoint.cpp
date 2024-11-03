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
size_t DataPoint::load_time(std::vector<DataPoint> previous_vertices) const {
    /// b_i = max{ b_i-1 + d_i-1 + c_i-1,i , e_i  }
    /// b_1 czas zakończenia rozładunku w poprzednim wierzchołku.
    ///d_i-1 czas rozładunku w poprzednim wierzchołku.
    ///c_i-1,i czas przejazdu między wierzchołkami i-1 i i
    ///e_i  najwcześniejszy dopuszczalny czas rozładunku w wierzchołku i


    let next_available_time = ready_time;

    if(previous_vertices.empty()) return next_available_time;

    let previous_vertex = previous_vertices.back();
    previous_vertices.pop_back();
    let finish_time =  previous_vertex.load_time(previous_vertices);
    let previous_reload_time = previous_vertex.getService();
    let distance =  static_cast<size_t>(get_distance(previous_vertex) );

    return std::max(finish_time + previous_reload_time + distance , next_available_time);
}
size_t DataPoint::load_time(size_t previous_load_time, const DataPoint & previous_vertex) const {
    /// b_i = max{ b_i-1 + d_i-1 + c_i-1,i , e_i  }
    /// b_1 czas zakończenia rozładunku w poprzednim wierzchołku.
    ///d_i-1 czas rozładunku w poprzednim wierzchołku.
    ///c_i-1,i czas przejazdu między wierzchołkami i-1 i i
    ///e_i  najwcześniejszy dopuszczalny czas rozładunku w wierzchołku i

    let next_available_time = ready_time;


    let finish_time = previous_load_time;
    let previous_reload_time = previous_vertex.getService();
    let distance =  static_cast<size_t>(get_distance(previous_vertex) );

    return std::max(finish_time + previous_reload_time + distance , next_available_time);
}


