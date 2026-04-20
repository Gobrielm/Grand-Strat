#pragma once

#include <classes/components/owner_component.hpp>
#include <classes/components/position_component.hpp>
#include <classes/components/capital_component.hpp>


using namespace godot;

class Station {

    enum TYPE_OF_STATION {
        NONE = 0,
        ROAD_DEPOT = 1,
        RAIL = 2
    };
    
    const int SUPPLY_DROPOFF;
    const int MAX_SUPPLY_DISTANCE;

    int get_SUPPLY_DROPOFF();
    int get_MAX_SUPPLY_DISTANCE();
    

    public:

    const TYPE_OF_STATION type;
    PositionComponent position;
    OwnerComponent owner;
    CapitalComponent capital;

    Station();
    Station(TYPE_OF_STATION p_type, std::pair<int, int> position, int owner);
    Station(Station& other);
    Station operator=(Station& other);

};