#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <godot_cpp/classes/ref_counted.hpp>

#include <classes/base_pop.hpp>

#include <classes/components/capital_component.hpp>
#include <classes/components/owner_component.hpp>
#include <classes/components/storage_component.hpp>
#include <classes/components/position_component.hpp>
#include <classes/components/construction_component.hpp>
#include <classes/components/town_components/market_component.hpp>

using namespace godot;

class Town: public RefCounted {
    GDCLASS(Town, RefCounted);

    std::unordered_map<int, std::vector<int>> internal_factories; // Owner id -> vector of factory_ids
    std::unordered_set<int> internal_companies; // Company ids
    std::unordered_set<int> town_pop_ids;

protected:
    static void _bind_methods();

public:

    PositionComponent position;
    OwnerComponent owner;

    MarketComponent mp;

    Town(std::pair<int, int> p_position = {0, 0});
    Town(Town& town);

    Town operator=(Town &town);

    void add_factory(int factory_owner, int factory_id);
    void add_company(int company_id);
    std::vector<int> get_factory_ids() const;

    //Pop stuff
    void add_pop(int pop_id);
    int get_total_pops() const;
};
