#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "classes/base_pop.hpp"

#include "classes/components/town_components/market_component.hpp"
#include "classes/components/position_component.hpp"
#include "classes/components/owner_component.hpp"

using namespace godot;

class Town {

    std::unordered_map<int, std::vector<int>> internal_factories; // Owner id -> vector of factory_ids
    std::unordered_set<int> internal_companies; // Company ids

protected:

public:

    PositionComponent position;
    OwnerComponent owner;

    MarketComponent mp;

    Town(std::pair<int, int> p_position = {0, 0});
    Town(const Town& town);

    Town& operator=(const Town &town);

    void add_factory(int factory_owner, int factory_id);
    void add_company(int company_id);
    std::vector<int> get_factory_ids() const;

    //Pop stuff
    void add_pop(int pop_id);
    int get_total_pops() const;
};
