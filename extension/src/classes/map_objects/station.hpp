#pragma once

#include "classes/components/capital_component.hpp"
#include "classes/components/owner_component.hpp"
#include "classes/components/position_component.hpp"


using namespace godot;

class Station {

    public:

    enum class TYPE_OF_STATION: int {
        NONE = 0,
        ROAD_DEPOT = 1,
        RAIL = 2
    };

    private:
    
    int SUPPLY_DROPOFF;
    int MAX_SUPPLY_DISTANCE;

    public:

    TYPE_OF_STATION type;
    PositionComponent position;
    OwnerComponent owner;
    CapitalComponent capital;

    Station();
    Station(TYPE_OF_STATION p_type, std::pair<int, int> position, int owner);
    Station(const Station& other);
    Station& operator=(const Station& other);

    int get_SUPPLY_DROPOFF();
    int get_MAX_SUPPLY_DISTANCE();
    
    static int get_SUPPLY_DROPOFF(TYPE_OF_STATION type);
    static int get_MAX_SUPPLY_DISTANCE(TYPE_OF_STATION type);
};