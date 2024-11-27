//
// Created by Wojciech on 01.11.2024.
//

#ifndef OK_GRASP_FLEETPROPERTIES_H
#define OK_GRASP_FLEETPROPERTIES_H
struct FleetProperties{
public:
    unsigned int vehicle_number;
    unsigned int capacity; // pojemność Q
public:
    FleetProperties(unsigned int vehicle_number,unsigned int capacity)
            :vehicle_number(vehicle_number),capacity(capacity){};
};

#endif //OK_GRASP_FLEETPROPERTIES_H
