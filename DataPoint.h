#ifndef OK_GRASP_DATAPOINT_H
#define OK_GRASP_DATAPOINT_H

#include <vector>
#include <algorithm>  // for std::swap

class DataPoint {
public:
    DataPoint(unsigned int customer, unsigned int x, unsigned int y, unsigned int demand,
              unsigned int ready, unsigned int due, unsigned int service);

    double get_distance(const DataPoint &dataPoint) const;

    unsigned int getCustomerNumber() const;

    unsigned int getService() const;
    unsigned int getDueDate() const;
    unsigned int getReadyTime() const;
    unsigned int getDemand() const;

    double load_time(double previous_load_time, const DataPoint &previous_vertex) const;

    // Friend swap function that swaps all members
    friend void swap(DataPoint& a, DataPoint& b) noexcept {
        std::swap(a.customer_number, b.customer_number);
        std::swap(a.x_coordinate, b.x_coordinate);
        std::swap(a.y_coordinate, b.y_coordinate);
        std::swap(a.demand, b.demand);
        std::swap(a.ready_time, b.ready_time);
        std::swap(a.due_date, b.due_date);
        std::swap(a.service, b.service);
    }

private:
    unsigned int customer_number;
    unsigned int x_coordinate;
    unsigned int y_coordinate;
    unsigned int demand;
    unsigned int ready_time;
    unsigned int due_date;
    unsigned int service;
};

#endif // OK_GRASP_DATAPOINT_H
