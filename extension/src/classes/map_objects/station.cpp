#include "station.hpp"

Station::Station(): type(TYPE_OF_STATION::NONE), SUPPLY_DROPOFF(1), MAX_SUPPLY_DISTANCE(0) {

}

Station::Station(TYPE_OF_STATION p_type, std::pair<int, int> p_position, int p_owner): 
    type(p_type), position(p_position, STATION), 
    SUPPLY_DROPOFF(get_SUPPLY_DROPOFF()), 
    MAX_SUPPLY_DISTANCE(get_MAX_SUPPLY_DISTANCE()
) {
    owner = OwnerComponent(p_owner);
}

Station::Station(const Station& other):
    position(other.position),
    owner(other.owner),
    MAX_SUPPLY_DISTANCE(other.MAX_SUPPLY_DISTANCE),
    SUPPLY_DROPOFF(other.SUPPLY_DROPOFF),
    type(other.type)
{}

Station& Station::operator=(const Station& other) {
    position = other.position;
    owner = other.owner;
    MAX_SUPPLY_DISTANCE = other.MAX_SUPPLY_DISTANCE;
    SUPPLY_DROPOFF = other.SUPPLY_DROPOFF;
    type = other.type;

    return *this;
}

int Station::get_SUPPLY_DROPOFF() {
    switch (type) {
        case TYPE_OF_STATION::RAIL:
            return 1;
        case TYPE_OF_STATION::ROAD_DEPOT:
            return 2;
        default:
            return 5;
    }

}

int Station::get_MAX_SUPPLY_DISTANCE() {
    switch (type) {
        case TYPE_OF_STATION::RAIL:
            return 30;
        case TYPE_OF_STATION::ROAD_DEPOT:
            return 12;
        default:
            return 5;
    }
}

int Station::get_SUPPLY_DROPOFF(TYPE_OF_STATION type) {
    switch (type) {
        case TYPE_OF_STATION::RAIL:
            return 30;
        case TYPE_OF_STATION::ROAD_DEPOT:
            return 12;
        default:
            return 5;
    }
}

int Station::get_MAX_SUPPLY_DISTANCE(TYPE_OF_STATION type) {
    switch (type) {
        case TYPE_OF_STATION::RAIL:
            return 30;
        case TYPE_OF_STATION::ROAD_DEPOT:
            return 12;
        default:
            return 5;
    }
}