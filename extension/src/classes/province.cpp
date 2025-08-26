#include "../singletons/terminal_map.hpp"
#include "../singletons/cargo_info.hpp"
#include "province.hpp"
#include "base_pop.hpp"
#include "terminal.hpp"
#include "factory_template.hpp"
#include "town.hpp"
#include "specific_buildings/subsistence_farm.hpp"
#include "factory_utility/recipe.hpp"
#include "../singletons/pop_manager.hpp"

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

    ClassDB::bind_method(D_METHOD("add_terminal", "tile"), &Province::add_terminal);
    ClassDB::bind_method(D_METHOD("remove_terminal", "tile"), &Province::remove_terminal);
    ClassDB::bind_method(D_METHOD("get_terminal_tiles"), &Province::get_terminal_tiles);

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
    std::unique_ptr<Recipe> peasant_recipe = SubsistenceFarm::get_recipe();
    float grain_o = (peasant_recipe->get_outputs().begin())->second;
    int pops_needed = peasant_recipe->get_pops_needed_num();
    auto stats = get_pop_type_statistics();
    std::scoped_lock lock(m);
    return (grain_o * stats[peasant]) / pops_needed;
}

float Province::get_demand_for_cargo(int type) const {
    auto stats = get_pop_type_statistics();
    float total_demand = 0;
    {
        std::scoped_lock lock(m);
        total_demand += stats[rural] * BasePop::get_base_need(rural, type); // Rural demand
        total_demand += stats[town] * BasePop::get_base_need(town, type); // Town demand
        total_demand += stats[peasant] * BasePop::get_base_need(peasant, type); // Peasant demand
    }
    return total_demand;
}

std::unordered_map<int, float> Province::get_demand_for_needed_goods() const {
    std::unordered_map<int, float> toReturn;
    std::unordered_map<PopTypes, size_t> pop_size = get_pop_type_statistics();;
    
    for (const auto& [type, amount]: BasePop::get_base_needs(rural)) {
        toReturn[type] += amount * pop_size[rural];   
    }
    for (const auto& [type, amount]: BasePop::get_base_needs(peasant)) {
        toReturn[type] += amount * pop_size[peasant];   
    }
    for (const auto& [type, amount]: BasePop::get_base_needs(town)) {
        toReturn[type] += amount * pop_size[town];   
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
    std::scoped_lock lock(m);
    return tiles;
}

std::vector<Vector2i> Province::get_town_centered_tiles() const { //Assumes one town
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<Vector2i> v;
    Vector2i town_tile;
    {
        std::scoped_lock lock(m);
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
        if (!terminal_tiles.count(tile)) {
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

void Province::add_terminal(Vector2i tile) {
    {
        std::scoped_lock lock(m);
        if (terminal_tiles.count(tile) != 0) {
            ERR_FAIL_MSG("Already has a terminal there");
            return;
        }
        terminal_tiles.insert(tile);
    }
    
    refresh_closest_town_to_tile();
}

void Province::remove_terminal(Vector2i tile) { //BUG: Never gets called when deleting terminals
    std::scoped_lock lock(m);
    if (terminal_tiles.count(tile) == 0) {
        ERR_FAIL_MSG("No terminal there");
        return;
    }
    terminal_tiles.erase(tile);
}
Array Province::get_terminal_tiles() const {
    Array a;
    std::scoped_lock lock(m);
    for (const auto tile: terminal_tiles) {
        a.push_back(tile);
    }
    return a;
}

bool Province::has_town() const {
    return get_town_tiles().size() != 0;
}

const std::unordered_set<Vector2i, godot_helpers::Vector2iHasher>& Province::get_terminal_tiles_set() const {
    return terminal_tiles;
}

void Province::refresh_closest_town_to_tile() {
    const auto& town_tiles = get_town_tiles();
    if (town_tiles.size() == 0) return; 
    std::scoped_lock lock(m);
    for (const Vector2i &tile: tiles) {
        Vector2i closest_town = get_closest_town_to_tile(tile, town_tiles);
        closest_town_to_tile[tile] = closest_town;
    }
}

Vector2i Province::get_closest_town_to_tile(Vector2i tile, std::vector<Vector2i> towns) {
    double closest_dist = -1;
    Vector2i closest_tile;
    for (const auto& cell: towns) {
        double temp_dist = tile.distance_to(cell);
        if (temp_dist < closest_dist || closest_dist == -1) {
            closest_dist = temp_dist;
            closest_tile = cell;
        }
    }
    return closest_tile;
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
        auto lock2 = pop_manager->lock_pop_read(id);
        stats[pop->get_type()]++;
    }
    return stats;
}


void Province::create_pops() {
    int number_of_peasant_pops = floor(population * 0.9 / BasePop::get_people_per_pop(peasant));
    int number_of_rural_pops = floor(population * 0.08 / BasePop::get_people_per_pop(rural));
	int number_of_city_pops = floor(population * 0.02 / BasePop::get_people_per_pop(town));
    for (int i = 0; i < number_of_peasant_pops; i++) {
        create_peasant_pop(0, tiles[rand() % tiles.size()]);
    }
    std::vector<Vector2i> towns = get_town_tiles();
    if (towns.size() == 0) {
		for (int i = 0; i < number_of_city_pops; i++) {
            create_peasant_pop(0, tiles[rand() % tiles.size()]);
        }
    }
    employ_peasants(); // Employ peasants before any other pops added to just look at peasants

	for (int i = 0; i < number_of_rural_pops; i++) {
        create_rural_pop(0, tiles[rand() % tiles.size()]);
    }
	
	//If no cities, then turn rest of population into peasant pops
	if (towns.size() != 0) {
        create_town_pops(number_of_city_pops, towns);
    }
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

void Province::create_town_pops(int amount, const std::vector<Vector2i>& towns) {
    int index = 0;
    auto terminal_map = TerminalMap::get_instance();
    std::vector<Ref<Town>> town_refs;
    for (const Vector2i &tile: towns) {
        town_refs.push_back(terminal_map->get_town(towns[index]));
    }

	for (int i = 0; i < amount; i++) {
        Ref<Town> town = town_refs[index];
        if (town.is_valid()) {
            int pop_id = create_town_pop(0, town -> get_location());
            town -> add_pop(pop_id);
        }
        
        index = (index + 1) % towns.size();
    }
}

int Province::create_town_pop(Variant culture, Vector2i p_location) {
    int pop_id = PopManager::get_instance()->create_pop(culture, p_location, town);
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
            if (closest_town_to_tile.count(tile))
                farm->set_local_town(closest_town_to_tile[tile]);
            
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
            auto lock2 = pop_manager->lock_pop_write(pop_id);
            if (pop->get_type() != peasant) continue;
            Ref<SubsistenceFarm> farm = terminal_map->get_terminal_as<SubsistenceFarm>(farms[i]);
            pop->set_location(farm->get_location());
            farm->add_pop(pop);

            i = (i + 1) % farms.size();
        }
    }
    
}

std::vector<Vector2i> Province::get_town_tiles() const {
    std::vector<Vector2i> toReturn;
    std::scoped_lock lock(m);
    for (const auto &tile: terminal_tiles) {
        if (TerminalMap::get_instance() -> is_town(tile)) {
            toReturn.push_back(tile);
        }
    }
    return toReturn;
}

int Province::count_pops() const {
    std::shared_lock lock(pops_lock);
    return pops.size();
}

Vector2i Province::get_closest_town_tile_to_pop(const Vector2i& pop_location) const {
    ERR_FAIL_COND_V_MSG(!closest_town_to_tile.count(pop_location), Vector2i(0, 0), "Pop doesn't have available town.");
    return closest_town_to_tile.at(pop_location);
}

bool Province::has_closest_town_tile_to_pop(const Vector2i& pop_location) const {
    return closest_town_to_tile.count(pop_location);
}