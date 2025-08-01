#include "../singletons/terminal_map.hpp"
#include "../singletons/cargo_info.hpp"
#include "province.hpp"
#include "base_pop.hpp"
#include "pops/rural_pop.hpp"
#include "pops/peasant_pop.hpp"
#include "town_pop.hpp"
#include "terminal.hpp"
#include "factory_template.hpp"
#include "town.hpp"
#include "specific_buildings/subsistence_farm.hpp"

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

    ClassDB::bind_method(D_METHOD("month_tick"), &Province::month_tick);
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
    for (const auto &[__, pop]: pops) {
        memdelete(pop);
    }
    pops.clear();
}

void Province::add_tile(Vector2i coords) {
    std::scoped_lock lock(m);
    tiles.push_back(coords);
}

int Province::get_population() const {
    std::scoped_lock lock(m);
    return population;
}

int Province::get_number_of_city_pops() const {
    std::shared_lock lock(pops_lock);
    return town_pops.size();
}

const std::unordered_set<int> Province::get_rural_pop_ids() const {
    std::shared_lock lock(pops_lock);
    return rural_pops;
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
    for (Vector2i tile: get_terminal_tiles_set()) {
        if (terminal_map->is_town(tile)) {
            town_tile = tile;
            break;
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

    for (Vector2i tile: get_tiles_vector()) {
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

double Province::get_total_wealth_of_pops() {
    double total = 0;
    std::shared_lock lock(pops_lock);
    for (const auto& [__, pop]: pops) {
        total += pop->get_wealth();
    }
    return total;
}

float Province::get_needs_met_of_pops() {
    double total = 0;
    std::shared_lock lock(pops_lock);
    for (const auto& [__, pop]: pops) {
        total += pop->get_average_fulfillment();
    }
    return total;
}

int Province::get_number_of_broke_pops() {
    int total = 0;
    std::shared_lock lock(pops_lock);
    for (const auto& [__, pop]: pops) {
        if (pop->get_wealth() < 15) {
            total++;
        }
    }
    return total;
}

void Province::pay_pop(int pop_id, float wage) {
    std::unique_lock lock(pops_lock);
    ERR_FAIL_COND_EDMSG(!pops.count(pop_id), "Tried to pay pop of unknown id: " + String::num(pop_id));
    pops[pop_id] -> pay_wage(wage);
}

void Province::fire_pop(int pop_id) {
    std::unique_lock lock(pops_lock);
    ERR_FAIL_COND_EDMSG(!pops.count(pop_id), "Tried to fire pop of unknown id: " + String::num(pop_id));
    pops[pop_id] -> fire();
}

BasePop* Province::get_pop(int pop_id) {
    ERR_FAIL_COND_V_EDMSG(!pops.count(pop_id), nullptr, "Pop of id: " + String::num(pop_id) + " not found.");
    return pops[pop_id];
}

void Province::create_pops() {
    int number_of_peasant_pops = floor(population * 0.6 / PeasantPop::get_people_per_pop());
    int number_of_rural_pops = floor(population * 0.2 / RuralPop::get_people_per_pop());
	int number_of_city_pops = floor(population * 0.2 / TownPop::get_people_per_pop());
    for (int i = 0; i < number_of_peasant_pops; i++) {
        create_peasant_pop(0, tiles[rand() % tiles.size()]);
    }
	for (int i = 0; i < number_of_rural_pops; i++) {
        create_rural_pop(0, tiles[rand() % tiles.size()]);
    }
	std::vector<Vector2i> towns = get_town_tiles();
	//If no cities, then turn rest of population into peasant pops
	if (towns.size() == 0) {
		for (int i = 0; i < number_of_city_pops; i++) {
            create_peasant_pop(0, tiles[rand() % tiles.size()]);
        }
    } else {
        create_town_pops(number_of_city_pops, towns);
    }

    // employ_peasants();

	
}

void Province::create_peasant_pop(Variant culture, Vector2i p_location) {
    PeasantPop* pop = memnew(PeasantPop(province_id, p_location, culture));
    {
        std::unique_lock lock(pops_lock);
        peasant_pops.insert(pop->get_pop_id());
        pops[pop->get_pop_id()] = pop;
    }
}

void Province::create_rural_pop(Variant culture, Vector2i p_location) {
    RuralPop* pop = memnew(RuralPop(province_id, p_location, culture));
    {
        std::unique_lock lock(pops_lock);
        rural_pops.insert(pop->get_pop_id());
        pops[pop->get_pop_id()] = pop;
    }
}

int Province::create_town_pop(Variant culture, Vector2i p_location) {
    TownPop* pop = memnew(TownPop(province_id, p_location, culture));
    {
        std::unique_lock lock(pops_lock);
        town_pops.insert(pop->get_pop_id());
        pops[pop->get_pop_id()] = pop;
    }
    return pop->get_pop_id();
}

void Province::create_town_pops(int amount, const std::vector<Vector2i>& towns) {
    int index = 0;
	for (int i = 0; i < amount; i++) {
        Ref<Town> town = TerminalMap::get_instance() -> get_town(towns[index]);
        
        if (town.is_valid()) {
            int pop_id = create_town_pop(0, town -> get_location());
            town -> add_pop(pop_id);
        }
        
        index = (index + 1) % towns.size();
    }
}

std::vector<int> Province::create_buildings_for_peasants() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<int> subsistence_farm_ids;
    for (const Vector2i &tile: tiles) {
        int temp = terminal_map->get_cargo_value_of_tile(tile, 10);
        if (temp > 0) {
            Ref<SubsistenceFarm> farm = Ref<SubsistenceFarm>(memnew(SubsistenceFarm(0)));
            terminal_map->create_isolated_terminal(farm);
            subsistence_farm_ids.push_back(farm->get_terminal_id());
        }
    }
    return subsistence_farm_ids;
}

void Province::employ_peasants() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<int> farms = create_buildings_for_peasants();
    if (farms.size() == 0) {
        print_error("No possible peasant buildings");
        return;
    }
    int i = 0;
    for (const auto& pop_id: peasant_pops) {
        BasePop* pop = get_pop(pop_id);
        Ref<SubsistenceFarm> farm = terminal_map->get_terminal_as<SubsistenceFarm>(farms[i]);
        pop->set_location(farm->get_location());
        farm->add_pop(pop); // Bug: Change to be pop id

        i = (i + 1) % farms.size();
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

void Province::find_employment_for_pops() {
    find_employment_for_rural_pops();
}

void Province::find_employment_for_town_pops() {
    //TODO: Create
}

void Province::find_employment_for_rural_pops() {
    Ref<FactoryTemplate> work;
    std::unique_lock lock(pops_lock);
    
    for (const auto &pop_id: rural_pops) {
        BasePop* pop = get_pop(pop_id);
        if (pop -> is_seeking_employment()) {
            if (work.is_valid() && work->is_hiring(pop)) {
                work->employ_pop(pop);
            } else {
                work = find_rural_employment(pop);
                if (work.is_valid()) {
                    work->employ_pop(pop);
                }
            }
        }
    }
}

Ref<FactoryTemplate> Province::find_rural_employment(BasePop* pop) const {
    float max_wage = 0.0;
    Ref<FactoryTemplate> best_fact = Ref<FactoryTemplate>(nullptr);
    
    for (const auto &tile: terminal_tiles) {
        Ref<FactoryTemplate> fact = TerminalMap::get_instance() -> get_terminal_as<FactoryTemplate>(tile);
        if (fact.is_null()) continue;

        float wage = fact -> get_wage();
        if (fact->is_hiring(pop) && wage > max_wage) {
            best_fact = fact;
            max_wage = wage;
        }

    }
	return best_fact;
}

Ref<FactoryTemplate> Province::find_urban_employment(BasePop* pop) const {
    float max_wage = 0.0;
    Ref<FactoryTemplate> best_fact = nullptr;
    for (const Vector2i& tile: get_town_tiles()) {
        Ref<Town> town = TerminalMap::get_instance() -> get_town(tile);

        if (town.is_null()) continue;

        Ref<FactoryTemplate> fact = town -> find_employment(pop);
        
        if (fact.is_null()) continue;
        
        if (fact -> get_wage() > max_wage) {
            best_fact = fact;
            max_wage = fact -> get_wage();
        }
    }
    return best_fact;
}

void Province::month_tick() {
    sell_to_pops();
    // find_employment_for_pops();
    // Employ or find education for peasants
}


void Province::sell_to_pops() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    // Get closest town and then use town functions to sell to those pops
    std::unique_lock lock(pops_lock);
    
    for (const auto& pop_id: town_pops) {
        BasePop* pop = get_pop(pop_id);
        pop->month_tick();
        Ref<Town> town = TerminalMap::get_instance()->get_town(pop->get_location());
        if (town.is_null()) {
            if (get_town_tiles().size() != 0) print_error("There are more than 0 towns"); 
            print_error("Pop is not in province or province has no towns, pop location: " + pop->get_location());
            continue;
        }
        
        town->sell_to_pop(pop);
    }

    for (const auto& pop_id: rural_pops) {
        BasePop* pop = get_pop(pop_id);
        pop->month_tick();
        if (!closest_town_to_tile.count(pop->get_location())) {
            // Rural pops may exist in province without town
            continue;
        }
        Ref<Town> town = TerminalMap::get_instance()->get_town(closest_town_to_tile[pop->get_location()]);
        if (town.is_null()) {
            print_error("Towns and terminals are desynced at " + pop->get_location());
            continue;
        }
        town->sell_to_pop(pop);
    }

}