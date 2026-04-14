#include "station.hpp"

void Station::_bind_methods() {

}

Station::Station(): type(NONE), SUPPLY_DROPOFF(1), MAX_SUPPLY_DISTANCE(0) {

}

Station::Station(TYPE_OF_STATION p_type, std::pair<int, int> p_position, int p_owner): 
    type(p_type), position(p_position, STATION), 
    SUPPLY_DROPOFF(get_SUPPLY_DROPOFF()), 
    MAX_SUPPLY_DISTANCE(get_MAX_SUPPLY_DISTANCE()
) {
    owner = OwnerComponent(p_owner);
}

int Station::get_SUPPLY_DROPOFF() {
    switch (type) {
        RAIL:
            return 1;
        ROAD:
            return 2;
        default:
            return 5;
    }

}

int Station::get_MAX_SUPPLY_DISTANCE() {
    switch (type) {
        RAIL:
            return 30;
        ROAD:
            return 12;
        default:
            return 5;
    }
}