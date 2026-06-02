#include "singletons/terminal_map.hpp"
#include "singletons/cargo_info.hpp"
#include "singletons/pop_manager.hpp"
#include "singletons/province_manager.hpp"

#include <iostream>
#include <queue>

#include "province.hpp"
#include "initial_builder.hpp"
#include "base_pop.hpp"
#include "map_objects/town.hpp"
#include "factory_utility/recipe.hpp"
#include "province_utility/province_pop_manager.h"

#include "classes/map_objects/subsistence_farm.hpp"
#include "classes/components/position_component.hpp"
#include "classes/components/town_components/market_component.hpp"
#include "classes/components/employer_component.hpp"
#include "classes/map_objects/station.hpp"

void Province::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_prov_id"), &Province::create);

    ClassDB::bind_method(D_METHOD("add_tile", "coords"), &Province::add_tile);
    ClassDB::bind_method(D_METHOD("get_tiles"), &Province::get_tiles);
    ClassDB::bind_method(D_METHOD("get_random_tile"), &Province::get_random_tile);

    ClassDB::bind_method(D_METHOD("add_population", "population_to_add"), &Province::add_population);
    ClassDB::bind_method(D_METHOD("set_population", "new_population"), &Province::set_population);
    ClassDB::bind_method(D_METHOD("get_population"), &Province::get_population);

    ClassDB::bind_method(D_METHOD("get_country_id"), &Province::get_country_id);
    ClassDB::bind_method(D_METHOD("get_province_id"), &Province::get_province_id);
    ClassDB::bind_method(D_METHOD("set_country_id", "country_id"), &Province::set_country_id);

    ClassDB::bind_method(D_METHOD("create_pops"), &Province::create_pops);
    ClassDB::bind_method(D_METHOD("get_number_of_pops"), &Province::get_number_of_pops);

    ClassDB::bind_method(D_METHOD("get_town_tile"), &Province::get_town_tile);
}
    
Province* Province::create(int p_prov_id) {
    return memnew(Province(p_prov_id));
}

void Province::initialize(int p_prov_id) {
    province_id = p_prov_id;
    population = 0;
}

Province::Province() {
    province_id = -1;
    population = 0;
}
Province::Province(int p_prov_id) {
    province_id = p_prov_id;
    population = 0;
}
Province::~Province() {
    
}

void Province::add_tile(Vector2i coords) {
    std::scoped_lock lock(m);
    tiles.push_back(coords);
}

int Province::get_population() const {
    std::scoped_lock lock(m);
    return population;
}

float Province::get_theoretical_supply_of_grain_from_peasants() const {
    EmployerComponent peasant_ec = SubsistenceFarm::get_default_employer_component();
    double grain_o = peasant_ec.recipe.get_outputs()[CargoInfo::get_instance()->get_cargo_type("grain")];
    int pops_needed = peasant_ec.get_pops_needed_num();
    auto stats = ppm.get_pop_type_statistics();
    return (grain_o * stats[PopTypes::peasant]) / pops_needed;
}

float Province::get_demand_for_cargo(int type) const {
    auto stats = ppm.get_pop_type_statistics();
    float total_demand = 0;
    {
        std::scoped_lock lock(m);
        total_demand += stats[PopTypes::rural] * BasePop::get_base_need(PopTypes::rural, type); // Rural demand
        total_demand += stats[PopTypes::town] * BasePop::get_base_need(PopTypes::town, type); // Town demand
        total_demand += stats[PopTypes::peasant] * BasePop::get_base_need(PopTypes::peasant, type); // Peasant demand
    }
    return total_demand;
}

std::unordered_map<int, float> Province::get_demand_for_needed_goods() const {
    std::unordered_map<int, float> toReturn;
    std::unordered_map<PopTypes, long> pop_size = ppm.get_pop_type_statistics();
    
    for (const auto& [type, amount]: BasePop::get_base_needs(PopTypes::rural)) {
        toReturn[type] += amount * pop_size[PopTypes::rural];   
    }
    for (const auto& [type, amount]: BasePop::get_base_needs(PopTypes::peasant)) {
        toReturn[type] += amount * pop_size[PopTypes::peasant];   
    }
    for (const auto& [type, amount]: BasePop::get_base_needs(PopTypes::town)) {
        toReturn[type] += amount * pop_size[PopTypes::town];   
    }

    toReturn[CargoInfo::get_instance()->get_cargo_type("grain")] -= get_theoretical_supply_of_grain_from_peasants();
    
    return toReturn;
}

void Province::add_population(int population_to_add) {
    std::scoped_lock lock(m);
    population += population_to_add;
}

void Province::set_population(int new_population) {
    std::scoped_lock lock(m);
    population = new_population;
}

int Province::get_province_id() const {
    return province_id;
}

int Province::get_country_id() const {
    std::scoped_lock lock(m);
    return country_id;
}

void Province::set_country_id(int p_country_id) {
    std::scoped_lock lock(m);
    country_id = p_country_id;
}

Array Province::get_tiles() const {
    Array a;
    std::scoped_lock lock(m);
    for (Vector2i tile: tiles) {
        a.append(tile);
    }
    return a;
}

const std::vector<Vector2i> Province::get_tiles_vector() const {
    return tiles;
}

std::vector<Vector2i> Province::get_town_centered_tiles() const { //Assumes one town
    std::vector<Vector2i> v;
    
    std::priority_queue<godot_helpers::weighted_value<Vector2i>,
        std::vector<godot_helpers::weighted_value<Vector2i>>, /*vector on backend*/
        std::greater<godot_helpers::weighted_value<Vector2i>> /*Smallest in front*/
    > pq;

    auto push = [&pq](Vector2i tile, int weight) -> void {
        pq.push(godot_helpers::weighted_value<Vector2i>(tile, weight));
    };

    auto town_tile = town.position.get_position_vector2i();

    for (Vector2i tile: tiles) {
        if (tile == town_tile) continue;
        push(tile, tile.distance_to(town_tile));
    }

    while (pq.size() != 0) {
        v.push_back(pq.top().val);
        pq.pop();
    }

    return v;
}

//Used to pick a place for random industries, don't pick places with industries
Vector2i Province::get_random_tile() const {
    std::vector<Vector2i> tiles_copy;
    {
        std::scoped_lock lock(m);
        for (Vector2i tile: tiles) {
            if (!position_components.count(tile)) {
                tiles_copy.push_back(tile);
            }
        }
    }
    
    if (tiles_copy.size() == 0) {
        return Vector2i(0, 0);
    } else if (tiles_copy.size() == 1) {
        return tiles_copy.at(0);
    }
    return tiles_copy.at(rand() % tiles_copy.size());
}

void Province::create_town() {
    std::scoped_lock lock(m);
    // Create Town
    // if (tiles.size() <= 2) {
    //     print_error("Province is too small.");
    // }
    Vector2i town_tile = tiles.at(rand() % tiles.size());
    town = Town(std::pair<int, int>(int(town_tile.x), int(town_tile.y)));

    id_to_vector_position[town.position.get_building_id()] = std::make_pair(0, town.position.get_type());
    position_components.emplace(town.position.get_position_vector2i(), town.position);
}

Factory& Province::add_factory(Factory& factory) {
    if (is_building_at_pos(factory.position.get_position_vector2i()) != 0) {
        print_error("Factory being placed in taken tile.");
        return factory;
    }

    std::scoped_lock lock(m);
    int spot = factories.size();
    factories.push_back(factory);
    id_to_vector_position[factory.position.get_building_id()] = std::make_pair(spot, factory.position.get_type());

    position_components.emplace(factory.position.get_position_vector2i(), factory.position);
    return factories.back();
}

Factory& Province::add_hidden_factory(Factory& factory) {
    if (is_building_at_pos(factory.position.get_position_vector2i()) == 0) {
        print_error("Placing Hidden Factory beneath nothing.");
        return factory;
    }

    std::scoped_lock lock(m);
    int spot = factories.size();
    factories.push_back(factory);
    id_to_vector_position[factory.position.get_building_id()] = std::make_pair(spot, factory.position.get_type());

    hidden_position_components[factory.position.get_position_vector2i()].push_back(factory.position);
    return factories.back();
}

Station& Province::add_station(Station& station) {
    if (is_building_at_pos(station.position.get_position_vector2i()) != 0) {
        print_error("Station being placed in taken tile.");
        return station;
    }

    std::scoped_lock lock(m);
    int spot = stations.size();
    stations.push_back(station);
    id_to_vector_position[station.position.get_building_id()] = std::make_pair(spot, station.position.get_type());

    position_components.emplace(station.position.get_position_vector2i(), station.position);
    return stations.back();
}

SubsistenceFarm& Province::add_subsistence_farm(SubsistenceFarm& farm) {
    std::scoped_lock lock(m);
    int spot = sub_farms.size();
    sub_farms.push_back(farm);
    id_to_vector_position[farm.position.get_building_id()] = std::make_pair(spot, farm.position.get_type());

    hidden_position_components[farm.position.get_position_vector2i()].push_back(farm.position);
    return sub_farms.back();
}

Factory& Province::get_factory(int pos_id) {
    std::scoped_lock(m);
    if (!id_to_vector_position.count(pos_id) || id_to_vector_position[pos_id].second != BuildingType::FACTORY) {
        std::cout << "Tried to fetch invalid factory with pos: "  + std::to_string(pos_id);
    }
    return factories[id_to_vector_position[pos_id].first];
}

Station& Province::get_station(int pos_id) {
    std::scoped_lock(m);
    if (!id_to_vector_position.count(pos_id) || id_to_vector_position[pos_id].second != BuildingType::STATION) {
        std::cout << "Tried to fetch invalid station with pos: "  + std::to_string(pos_id);
    }
    return stations[id_to_vector_position[pos_id].first];
}

Town& Province::get_town() {
    return town;
}

bool Province::is_building_at_pos(Vector2i pos) const {
    std::scoped_lock lock(m);
    return position_components.count(pos);
}

int Province::get_hidden_buildings_at_pos(Vector2i pos) const {
    std::scoped_lock lock(m);
    if (hidden_position_components.count(pos)) {
        return hidden_position_components.at(pos).size();
    }
    return 0;
}

BuildingType Province::get_building_type(int pos_id) const {
    if (!id_to_vector_position.count(pos_id)) return BuildingType::INVALID;
    return id_to_vector_position.at(pos_id).second;
}

BuildingType Province::get_visible_building_type(Vector2i tile) const {
    std::scoped_lock lock(m);
    if (position_components.count(tile)) {
        return position_components.at(tile).get_type();
    }
    return BuildingType::INVALID;
}

PositionComponent Province::get_visible_position_component(Vector2i tile) const {
    std::scoped_lock lock(m);
    if (position_components.count(tile)) {
        return position_components.at(tile);
    }
    return PositionComponent();
}

// void Province::add_terminal(Vector2i tile) {
//     {
//         std::scoped_lock lock(m);
//         if (terminal_tiles.count(tile) != 0) {
//             ERR_FAIL_MSG("Already has a terminal there");
//             return;
//         }
//         terminal_tiles.insert(tile);
//     }
    
//     refresh_closest_town_to_tile();
// }

// void Province::remove_terminal(Vector2i tile) { //BUG: Never gets called when deleting terminals
//     std::scoped_lock lock(m);
//     if (terminal_tiles.count(tile) == 0) {
//         ERR_FAIL_MSG("No terminal there");
//         return;
//     }
//     terminal_tiles.erase(tile);
// }
// Array Province::get_terminal_tiles() const {
//     Array a;
//     std::scoped_lock lock(m);
//     for (const auto tile: terminal_tiles) {
//         a.push_back(tile);
//     }
//     return a;
// }

bool Province::has_town() const {
    return true;
}

Vector2i Province::get_town_tile() const {
    return town.position.get_position_vector2i();
}

void Province::init_province() {
    auto tm = TerminalMap::get_instance();

    // encode previously created town
    tm->encode_building(town.position);

    // Create factories
    
}

int Province::get_number_of_pops() const {
    std::scoped_lock lock(m);
    return ppm.get_number_of_pops();
}

void Province::create_pops() {
    int number_of_peasant_pops = floor(population * 0.9 / BasePop::get_people_per_pop(PopTypes::peasant));
    int number_of_rural_pops = floor(population * 0.08 / BasePop::get_people_per_pop(PopTypes::rural));
	int number_of_city_pops = floor(population * 0.02 / BasePop::get_people_per_pop(PopTypes::town));

    if (tiles.size() == 0) {
        print_error("No Tiles in Province.");
        return;
    }

    for (int i = 0; i < number_of_peasant_pops; i++) {
        Vector2i rand_tile = tiles[rand() % tiles.size()];
        if (rand_tile == Vector2i(0, 0)) {
            print_error("Broken Tile in Province.");
            return;
        }
        create_peasant_pop(0, rand_tile);
    }

    employ_peasants(); // Employ peasants before any other pops added to just look at peasants

	for (int i = 0; i < number_of_rural_pops; i++) {
        Vector2i rand_tile = tiles[rand() % tiles.size()];
        if (rand_tile == Vector2i(0, 0)) {
            print_error("Broken Tile in Province.");
            return;
        }
        create_rural_pop(0, rand_tile);
    }
	
    create_town_pops(number_of_city_pops);
}

void Province::create_peasant_pop(Variant culture, Vector2i p_location) {
    std::unique_lock lock(m);
    ppm.create_pop(PopTypes::peasant, culture, p_location, province_id);
}

void Province::create_rural_pop(Variant culture, Vector2i p_location) {
    std::unique_lock lock(m);
    ppm.create_pop(PopTypes::rural, culture, p_location, province_id);
}

void Province::create_town_pops(int amount) {
    Vector2i town_pos = town.position.get_position_vector2i();
    std::unique_lock lock(m);
	for (int i = 0; i < amount; i++) {
        ppm.create_pop(PopTypes::town, 0, town_pos, province_id);
    }
}

int Province::create_town_pop(Variant culture, Vector2i p_location) {
    std::unique_lock lock(m);
    return ppm.create_pop(PopTypes::town, 0, p_location, province_id);
}

void Province::create_buildings_for_peasants() {
    auto terminal_map = TerminalMap::get_instance();

    for (const Vector2i &tile: tiles) {
        int temp = terminal_map->get_cargo_value_of_tile(tile, CargoInfo::get_instance()->get_cargo_type("grain"));
        if (temp > 0) {
            SubsistenceFarm farm(tile, 0);
            farm = add_subsistence_farm(farm);
            terminal_map->encode_building(farm.position);
        }
    }
}

void Province::employ_peasants() {
    if (ppm.get_number_of_pops() == 0) {
        return;
    }

    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    
    std::scoped_lock(m);
    create_buildings_for_peasants();
    
    if (sub_farms.size() == 0) {
        // print_line(tiles.front());
        // print_error("No possible peasant buildings");
        return;
    }
    
    int i = 0;

    for (auto& [id, pop]: ppm.pops) {
        if (pop.get_type() != PopTypes::peasant) continue;
        
        SubsistenceFarm& farm = sub_farms[i];
        
        farm.add_pop(town, &pop);          

        i = (i + 1) % sub_farms.size();
    }
    
    
}

Factory& Province::get_factory_unsafe(int pos_id) {
    if (!id_to_vector_position.count(pos_id) || id_to_vector_position[pos_id].second != BuildingType::FACTORY) {
        std::cout << "Tried to fetch invalid factory with pos: "  + std::to_string(pos_id);
    }
    return factories[id_to_vector_position[pos_id].first];
}

Station& Province::get_station_unsafe(int pos_id) {
    if (!id_to_vector_position.count(pos_id) || id_to_vector_position[pos_id].second != BuildingType::STATION) {
        std::cout << "Tried to fetch invalid station with pos: "  + std::to_string(pos_id);
    }
    return stations[id_to_vector_position[pos_id].first];
}

PositionComponent Province::get_visible_position_component_unsafe(Vector2i tile) const {
    if (position_components.count(tile)) {
        return position_components.at(tile);
    }
    return PositionComponent();
}

std::pair<CapitalComponent*, StorageComponent*> Province::get_capital_and_storage_components_unsafe(const TradeOrder& order) {
    int source_id = order.get_source_id();

    auto get_components_factory = [&] () {
        if (!id_to_vector_position.count(source_id)) return std::pair<CapitalComponent*, StorageComponent*>(nullptr, nullptr);
        const auto type = id_to_vector_position.at(source_id).second;

        switch (type) {
            case BuildingType::FACTORY: {
                auto& factory = factories[id_to_vector_position[source_id].first];
                return std::pair<CapitalComponent*, StorageComponent*>(&factory.capital, &factory.storage);
            }
            case BuildingType::SUBSISTENCE_FARM: {
                auto& sub_farm = sub_farms[id_to_vector_position[source_id].first];
                return std::pair<CapitalComponent*, StorageComponent*>(&sub_farm.capital, &sub_farm.storage);
            }
            default: {
                print_error("Unknown Entity Tried to Trade: " + String::num(static_cast<int>(type)));
                return std::pair<CapitalComponent*, StorageComponent*>(nullptr, nullptr);
            }
        }
    };

    auto get_components_pop = [&] () {
        if (ppm.pops.count(source_id)) {
            BasePop* pop = ppm.get_pop(source_id);

            return std::pair<CapitalComponent*, StorageComponent*>(&pop->capital, &pop->storage);
        } else {
            print_error("Unowned pop is being accessed in province: " + String::num(province_id));
            return std::pair<CapitalComponent*, StorageComponent*>(nullptr, nullptr);
        }
    };


    switch (order.get_owner_type()) {

        case TradeOrderOwner::BUILDING:
            return get_components_factory();
        
        case TradeOrderOwner::POP:
            return get_components_pop();

        default:
            print_error("Accessing capital/storage of unknown order owner: " + String::num(int(order.get_owner_type())));
            
    }   
    return std::make_pair(nullptr, nullptr);
}