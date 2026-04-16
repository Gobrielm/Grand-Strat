#include "../singletons/terminal_map.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/trading_system.hpp"

#include "province.hpp"
#include "base_pop.hpp"
#include "terminal.hpp"
#include "factory_template.hpp"
#include "town.hpp"
#include "factory_utility/recipe.hpp"
#include "../singletons/pop_manager.hpp"

#include <classes/map_objects/subsistence_farm.hpp>
#include <classes/components/town_components/market_component.hpp>
#include <classes/components/town_components/employer_component.hpp>
#include <classes/map_objects/station.hpp>

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
    ClassDB::bind_method(D_METHOD("count_pops"), &Province::count_pops);
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
Province::Province(int p_prov_id): town(create_town()) {
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
    std::shared_lock lock(m);
    return population;
}

float Province::get_theoretical_supply_of_grain_from_peasants() const {
    std::unique_ptr<Recipe> peasant_recipe = SubsistenceFarm::get_recipe();
    float grain_o = (peasant_recipe->get_outputs().begin())->second;
    int pops_needed = peasant_recipe->get_pops_needed_num();
    auto stats = get_pop_type_statistics();
    std::shared_lock lock(m);
    return (grain_o * stats[peasant]) / pops_needed;
}

float Province::get_demand_for_cargo(int type) const {
    auto stats = get_pop_type_statistics();
    float total_demand = 0;
    {
        std::shared_lock lock(m);
        total_demand += stats[PopTypes::rural] * BasePop::get_base_need(PopTypes::rural, type); // Rural demand
        total_demand += stats[PopTypes::town] * BasePop::get_base_need(PopTypes::town, type); // Town demand
        total_demand += stats[PopTypes::peasant] * BasePop::get_base_need(PopTypes::peasant, type); // Peasant demand
    }
    return total_demand;
}

std::unordered_map<int, float> Province::get_demand_for_needed_goods() const {
    std::unordered_map<int, float> toReturn;
    std::unordered_map<PopTypes, size_t> pop_size = get_pop_type_statistics();;
    
    for (const auto& [type, amount]: BasePop::get_base_needs(PopTypes::rural)) {
        toReturn[type] += amount * pop_size[PopTypes::rural];   
    }
    for (const auto& [type, amount]: BasePop::get_base_needs(PopTypes::peasant)) {
        toReturn[type] += amount * pop_size[PopTypes::peasant];   
    }
    for (const auto& [type, amount]: BasePop::get_base_needs(PopTypes::town)) {
        toReturn[type] += amount * pop_size[PopTypes::town];   
    }

    toReturn[10] -= get_theoretical_supply_of_grain_from_peasants();
    
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
    std::shared_lock lock(m);
    return country_id;
}

void Province::set_country_id(int p_country_id) {
    std::scoped_lock lock(m);
    country_id = p_country_id;
}

Array Province::get_tiles() const {
    Array a;
    std::shared_lock lock(m);
    for (Vector2i tile: tiles) {
        a.append(tile);
    }
    return a;
}

const std::vector<Vector2i> Province::get_tiles_vector() const {
    std::shared_lock lock(m);
    return tiles;
}

std::vector<Vector2i> Province::get_town_centered_tiles() const { //Assumes one town
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<Vector2i> v;
    Vector2i town_tile;
    {
        std::shared_lock lock(m);
        for (Vector2i tile: terminal_tiles) {
            if (terminal_map->is_town(tile)) {
                town_tile = tile;
                break;
            }
        }
    }
    
    if (town_tile == Vector2i(0, 0)) {
        ERR_FAIL_V_MSG(v, "No town in province");
    }
    std::priority_queue<godot_helpers::weighted_value<Vector2i>,
    std::vector<godot_helpers::weighted_value<Vector2i>>, /*vector on backend*/
    std::greater<godot_helpers::weighted_value<Vector2i>> /*Smallest in front*/
    > pq;

    auto push = [&pq](Vector2i tile, int weight) -> void {pq.push(godot_helpers::weighted_value<Vector2i>(tile, weight));};

    for (Vector2i tile: tiles) {
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
    m.lock();
    for (Vector2i tile: tiles) {
        if (!position_components.count(tile)) {
            tiles_copy.push_back(tile);
        }
    }
    m.unlock();
    if (tiles_copy.size() == 0) {
        return Vector2i(0, 0);
    } else if (tiles_copy.size() == 1) {
        return tiles_copy.at(0);
    }
    return tiles_copy.at(rand() % (tiles_copy.size() - 1));
}

Town Province::create_town() {
    // Create Town
    Vector2i town_tile = tiles[rand() % tiles.size()];
    return Town(std::make_pair(town_tile.x, town_tile.y));
}

void Province::add_factory(Factory& factory) {
    std::scoped_lock lock(m);
    int spot = factories.size();
    factories.push_back(factory);
    id_to_vector_position[factory.position.building_id] = std::make_pair(spot, factory.position.type);

    position_components[factory.position.get_position_vector2i()].push_back(factory.position);
}

void Province::add_town(Town& p_town) {
    std::scoped_lock lock(m);
    town = p_town;
    ERR_FAIL_COND_MSG(position_components[town.position.get_position_vector2i()].size() != 0, "Putting town in taken tile.");
    position_components[town.position.get_position_vector2i()].push_back(town.position);
}

void Province::add_station(Station& station) {
    std::scoped_lock lock(m);
    stations[station.position.building_id] = station;
    position_components[station.position.get_position_vector2i()].push_back(station.position);
}

Factory& Province::get_factory(int pos_id) {
    std::scoped_lock lock(m);
    if (!id_to_vector_position.count(pos_id) || id_to_vector_position[pos_id].second != BuildingType::FACTORY) {
        Factory factory;
        ERR_FAIL_V_MSG(factory, "Tried to fetch invalid factory with pos: "  + String(std::to_string(pos_id).c_str()));
    }
    return factories[id_to_vector_position[pos_id].first];
}

Station& Province::get_station(int pos_id) {
    std::scoped_lock lock(m);
    if (!id_to_vector_position.count(pos_id) || id_to_vector_position[pos_id].second != BuildingType::STATION) {
        Station station;
        ERR_FAIL_V_MSG(station, "Tried to fetch invalid station with pos: "  + String(std::to_string(pos_id).c_str()));
    }
    return stations[id_to_vector_position[pos_id].first];
}

Town& Province::get_town() {
    return town;
}

BuildingType Province::get_building_type(int pos_id) const {
    std::scoped_lock lock(m);
    if (!id_to_vector_position.count(pos_id)) return BuildingType::INVALID;
    return id_to_vector_position.at(pos_id).second;
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
//     std::shared_lock lock(m);
//     for (const auto tile: terminal_tiles) {
//         a.push_back(tile);
//     }
//     return a;
// }

bool Province::has_town() const {
    return true;
}

void Province::init_province() {
    auto tm = TerminalMap::get_instance();

    // encode previously created town
    tm->encode_building(town.position);

    // Create factories
    
}

int Province::get_number_of_pops() const {
    std::shared_lock lock(pops_lock);
    return pops.size();
}

std::unordered_map<PopTypes, size_t> Province::get_pop_type_statistics() const {
    auto pop_manager = PopManager::get_instance();
    std::unordered_map<PopTypes, size_t> stats;
    std::unordered_set<int> pops_copy;
    {
        std::shared_lock lock(pops_lock);
        pops_copy = pops;
    }
    
    for (const auto& id: pops_copy) {
        auto pop = pop_manager->get_pop(id);
        auto lock = pop_manager->lock_pop_read(id);
        stats[pop->get_type()]++;
    }
    return stats;
}


void Province::create_pops() {
    int number_of_peasant_pops = floor(population * 0.9 / BasePop::get_people_per_pop(PopTypes::peasant));
    int number_of_rural_pops = floor(population * 0.08 / BasePop::get_people_per_pop(PopTypes::rural));
	int number_of_city_pops = floor(population * 0.02 / BasePop::get_people_per_pop(PopTypes::town));
    for (int i = 0; i < number_of_peasant_pops; i++) {
        create_peasant_pop(0, tiles[rand() % tiles.size()]);
    }
    
    employ_peasants(); // Employ peasants before any other pops added to just look at peasants

	for (int i = 0; i < number_of_rural_pops; i++) {
        create_rural_pop(0, tiles[rand() % tiles.size()]);
    }
	
	//If no cities, then turn rest of population into peasant pops
    create_town_pops(number_of_city_pops);
}

void Province::create_peasant_pop(Variant culture, Vector2i p_location) {
    int pop_id = PopManager::get_instance()->create_pop(culture, p_location, peasant);
    {
        std::unique_lock lock(pops_lock);
        pops.insert(pop_id);
    }
}

void Province::create_rural_pop(Variant culture, Vector2i p_location) {
    int pop_id = PopManager::get_instance()->create_pop(culture, p_location, rural);
    {
        std::unique_lock lock(pops_lock);
        pops.insert(pop_id);
    }
}

void Province::create_town_pops(int amount) {
    int index = 0;

	for (int i = 0; i < amount; i++) {
        int pop_id = create_town_pop(0, town.position.get_position_vector2i());
        town.add_pop(pop_id);
    }
}

int Province::create_town_pop(Variant culture, Vector2i p_location) {
    int pop_id = PopManager::get_instance()->create_pop(culture, p_location, PopTypes::town);
    {
        std::unique_lock lock(pops_lock);
        pops.insert(pop_id);
    }
    return pop_id;
}

std::vector<int> Province::create_buildings_for_peasants() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<int> subsistence_farm_ids;
    for (const Vector2i &tile: tiles) {
        int temp = terminal_map->get_cargo_value_of_tile(tile, 10);
        if (temp > 0) {
            Ref<SubsistenceFarm> farm = Ref<SubsistenceFarm>(memnew(SubsistenceFarm(tile, 0)));
            farm->set_local_town(town.position.get_position_vector2i());
            
            terminal_map->create_isolated_terminal(farm);
            subsistence_farm_ids.push_back(farm->get_terminal_id());
        }
    }
    return subsistence_farm_ids;
}

void Province::employ_peasants() {
    if (pops.size() == 0) {
        return;
    }

    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    auto pop_manager = PopManager::get_instance();
    std::vector<int> farms = create_buildings_for_peasants();
    if (farms.size() == 0) {
        // print_line(tiles.front());
        // print_error("No possible peasant buildings");
        return;
    }
    
    {
        int i = 0;
        std::unordered_set<int> pops_copy;
        {
            std::shared_lock lock(pops_lock);
            pops_copy = pops;
        }

        for (const auto& pop_id: pops_copy) {
            auto pop = pop_manager->get_pop(pop_id);
            auto lock = pop_manager->lock_pop_write(pop_id);
            if (pop->get_type() != peasant) continue;
            Ref<SubsistenceFarm> farm = terminal_map->get_terminal_as<SubsistenceFarm>(farms[i]);
            pop->set_location(farm->get_location());
            farm->add_pop(pop);

            i = (i + 1) % farms.size();
        }
    }
    
}

int Province::count_pops() const {
    std::shared_lock lock(pops_lock);
    return pops.size();
}