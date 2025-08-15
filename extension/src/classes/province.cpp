#define CONST_POP_LOOP() \
    for (const auto& [__, pop] : pops)

#define CONST_TOWN_POP_LOOP() \
    for (const auto& pop_id : pop_types[town])
#define CONST_RURAL_POP_LOOP() \
    for (const auto& pop_id : pop_types[rural])
#define CONST_PEASANT_POP_LOOP() \
    for (const auto& pop_id : pop_types[peasant])

#include "../singletons/terminal_map.hpp"
#include "../singletons/cargo_info.hpp"
#include "province.hpp"
#include "base_pop.hpp"
#include "terminal.hpp"
#include "factory_template.hpp"
#include "town.hpp"
#include "specific_buildings/subsistence_farm.hpp"
#include "factory_utility/recipe.hpp"

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
    CONST_POP_LOOP() {
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

float Province::get_theoretical_supply_of_grain_from_peasants() const {
    std::unique_ptr<Recipe> peasant_recipe = SubsistenceFarm::get_recipe();
    float grain_o = (peasant_recipe->get_outputs().begin())->second;
    int pops_needed = peasant_recipe->get_pops_needed_num();

    std::scoped_lock lock(m);
    return (grain_o * pop_types.at(peasant).size()) / pops_needed;
}

float Province::get_demand_for_cargo(int type) const {
    float total_demand = 0;
    {
        std::scoped_lock lock(m);
        total_demand += pop_types.at(rural).size() * BasePop::get_base_need(rural, type); // Rural demand
        total_demand += pop_types.at(town).size() * BasePop::get_base_need(town, type); // Town demand
        total_demand += pop_types.at(peasant).size() * BasePop::get_base_need(peasant, type); // Peasant demand
    }
    return total_demand;
}

std::unordered_map<int, float> Province::get_demand_for_needed_goods() const {
    std::unordered_map<int, float> toReturn;
    std::unordered_map<PopTypes, size_t> pop_size = {
        {rural, pop_types.at(rural).size()},
        {town, pop_types.at(town).size()},
        {peasant, pop_types.at(peasant).size()}
    };
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

double Province::get_total_wealth_of_pops() const {
    double total = 0;
    std::shared_lock lock(pops_lock);
    CONST_POP_LOOP() {
        total += pop->get_wealth();
    }
    return total;
}

float Province::get_needs_met_of_pops() const {
    double total = 0;
    std::shared_lock lock(pops_lock);
    CONST_POP_LOOP() {
        total += pop->get_average_fulfillment();
    }
    return total;
}

int Province::get_number_of_broke_pops() const {
    int total = 0;
    std::shared_lock lock(pops_lock);
    CONST_POP_LOOP() {
        if (pop->get_wealth() < 15) {
            total++;
        }
    }
    return total;
}

int Province::get_number_of_starving_pops() const {
    int total = 0;
    std::shared_lock lock(pops_lock);
    CONST_POP_LOOP() {
        if (pop->is_starving()) {
            total++;
        }
    }
    return total;
}

int Province::get_number_of_unemployed_pops() const {
    int total = 0;
    std::shared_lock lock(pops_lock);
    CONST_POP_LOOP() {
        if (pop->is_seeking_employment()) {
            total++;
        }
    }
    return total;
}

int Province::get_number_of_peasants() const {
    std::shared_lock lock(pops_lock);
    if (pop_types.count(peasant)) {
        return pop_types.at(peasant).size();
    }
    return 0;
}

int Province::get_number_of_city_pops() const {
    std::shared_lock lock(pops_lock);
    if (pop_types.count(town)) {
        return pop_types.at(town).size();
    }
    return 0;
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

void Province::sell_cargo_to_pop(int pop_id, int type, int amount, float price) {
    std::unique_lock lock(pops_lock);
    BasePop* pop = get_pop(pop_id);
    pop->buy_good(type, amount, price);
}

int Province::get_pop_desired(int pop_id, int type, float price) {
    std::unique_lock lock(pops_lock);
    BasePop* pop = get_pop(pop_id);
    return pop->get_desired(type, price);
}

BasePop* Province::get_pop(int pop_id) {
    ERR_FAIL_COND_V_EDMSG(!pops.count(pop_id), nullptr, "Pop of id: " + String::num(pop_id) + " not found.");
    return pops[pop_id];
}

void Province::create_pops() {
    int number_of_peasant_pops = floor(population * 0.9 / BasePop::get_people_per_pop(peasant));
    int number_of_rural_pops = floor(population * 0.05 / BasePop::get_people_per_pop(rural));
	int number_of_city_pops = floor(population * 0.05 / BasePop::get_people_per_pop(town));
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

    employ_peasants();

	
}

void Province::create_peasant_pop(Variant culture, Vector2i p_location) {
    BasePop* pop = BasePop::create_peasant_pop(province_id, p_location, culture);
    {
        std::unique_lock lock(pops_lock);
        pop_types[peasant].insert(pop->get_pop_id());
        pops[pop->get_pop_id()] = pop;
    }
}

void Province::create_rural_pop(Variant culture, Vector2i p_location) {
    BasePop* pop = BasePop::create_rural_pop(province_id, p_location, culture);
    {
        std::unique_lock lock(pops_lock);
        pop_types[rural].insert(pop->get_pop_id());
        pops[pop->get_pop_id()] = pop;
    }
}

int Province::create_town_pop(Variant culture, Vector2i p_location) {
    BasePop* pop = BasePop::create_town_pop(province_id, p_location, culture);
    {
        std::unique_lock lock(pops_lock);
        pop_types[town].insert(pop->get_pop_id());
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
            Ref<SubsistenceFarm> farm = Ref<SubsistenceFarm>(memnew(SubsistenceFarm(tile, 0)));
            farm->set_local_town(closest_town_to_tile[tile]);
            
            terminal_map->create_isolated_terminal(farm);
            subsistence_farm_ids.push_back(farm->get_terminal_id());
        }
    }
    return subsistence_farm_ids;
}

void Province::employ_peasants() {
    if (get_town_tiles().size() == 0) {
        return;
    }

    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<int> farms = create_buildings_for_peasants();
    if (farms.size() == 0) {
        // print_line(tiles.front());
        // print_error("No possible peasant buildings");
        return;
    }
    int i = 0;
    for (const auto& pop_id: pop_types[peasant]) {
        BasePop* pop = get_pop(pop_id);
        Ref<SubsistenceFarm> farm = terminal_map->get_terminal_as<SubsistenceFarm>(farms[i]);
        pop->set_location(farm->get_location());
        farm->add_pop(pop);

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
    find_employment_for_town_pops();
}

void Province::find_employment_for_rural_pops() {
    auto s = get_rural_employment_sorted_by_wage();
    auto it = s.begin();
    if (it == s.end()) {
        // No Available factories hiring
        return;
    }

    {
        Ref<FactoryTemplate> work = *it;
        std::unique_lock lock(pops_lock);
        for (const auto &pop_id: pop_types[rural]) {
            BasePop* pop = get_pop(pop_id);
            if (pop -> is_seeking_employment()) {
                while (!work->is_hiring(rural)) {
                    it++;
                    if (it == s.end()) return;
                    work = *it;
                }

                if (pop->is_wage_acceptable(work->get_wage())) {
                    work->employ_pop(pop);
                }
            }
        }
    }
}

void Province::find_employment_for_town_pops() {
    // Gets the employement opputunities for each town as they will be needed by pops in those towns
    std::unordered_map<Vector2i, std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare>, godot_helpers::Vector2iHasher> m;
    std::unordered_map<Vector2i, std::set<godot::Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare>::iterator, godot_helpers::Vector2iHasher> it_map;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> towns_with_employement;
    for (const auto& tile: get_town_tiles()) {
        m[tile] = get_town_employment_sorted_by_wage(tile);
        it_map[tile] = m[tile].begin();
        if (it_map[tile] != m[tile].end()) {
            // Available factories hiring
            towns_with_employement.insert(tile);
        }
    }
    if (towns_with_employement.size() == 0) return;
    Ref<FactoryTemplate> work;
    std::unique_lock lock(pops_lock);
    
    for (const auto &pop_id: pop_types[town]) {
        BasePop* pop = get_pop(pop_id);
        if (pop -> is_seeking_employment()) {
            Vector2i pop_location = pop->get_location();
            if (!m.count(pop_location)) {
                ERR_FAIL_MSG("Town Pop not located in town or province doesn't have town at " + pop_location);
            }
            if (!towns_with_employement.count(pop_location)) continue;

            auto& it = it_map[pop_location];

            work = *(it);
            if (work.is_null()) {
                ERR_FAIL_MSG("Factory is null");
            }
            while (!work->is_hiring(town)) {
                it++;
                if (it == m[pop_location].end()) {
                    towns_with_employement.erase(pop_location);
                    if (towns_with_employement.size() == 0) return;
                    break;
                }
                work = *(it);
            }
            if (!towns_with_employement.count(pop_location)) continue;

            if (pop->is_wage_acceptable(work->get_wage())) {
                print_line("Employed");
                work->employ_pop(pop);
            }
        }
    }
}

std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare> Province::get_rural_employment_sorted_by_wage() const {
    std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare> s;
    for (const auto &tile: terminal_tiles) {
        Ref<FactoryTemplate> fact = TerminalMap::get_instance() -> get_terminal_as<FactoryTemplate>(tile);
        if (fact.is_null()) continue;

        float wage = fact -> get_wage();
        if (fact->is_hiring(rural)) {
            s.insert(fact);
        }
    }
    return s;
}

std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare> Province::get_town_employment_sorted_by_wage(Vector2i town_location) const {
    Ref<Town> town_ref = TerminalMap::get_instance()->get_town(town_location);
    ERR_FAIL_COND_V_MSG(town_ref.is_null(), (std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare>()), "Location sent is to a null town");
    return town_ref->get_employment_sorted_by_wage(town);
}

Ref<Town> Province::get_closest_town_to_pop(BasePop* pop) const {
    return TerminalMap::get_instance()->get_town(closest_town_to_tile.at(pop->get_location()));
}

void Province::month_tick() {
    sell_to_pops();
    change_pops();
    find_employment_for_pops();
    // Employ or find education for peasants
}

void Province::change_pops() {
    std::unique_lock lock(pops_lock);
    auto& rural_set = pop_types[rural];
    for (auto it = rural_set.begin(); it != rural_set.end();) {
        int pop_id = *it;
        BasePop* pop = get_pop(pop_id);
        if (pop->will_degrade()) {
            pop->degrade();
            pop_types[peasant].insert(pop_id);
            it = rural_set.erase(it);
        } else {
            it++;
        }
    }
    auto& peasant_set = pop_types[peasant];
    for (auto it = peasant_set.begin(); it != peasant_set.end();) {
        int pop_id = *it;
        BasePop* pop = get_pop(pop_id);
        if (pop->will_upgrade()) {
            pop->upgrade();
            pop_types[peasant].insert(pop_id);
            it = peasant_set.erase(it);
        } else {
            it++;
        }
    }
}


void Province::sell_to_pops() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    // Get closest town and then use town functions to sell to those pops
    std::unique_lock lock(pops_lock);
    
    CONST_TOWN_POP_LOOP() {
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

    CONST_RURAL_POP_LOOP() {
        BasePop* pop = get_pop(pop_id);
        pop->month_tick();
        if (!closest_town_to_tile.count(pop->get_location())) {
            // Rural pops may exist in province without town
            continue;
        }
        Ref<Town> town = get_closest_town_to_pop(pop);
        if (town.is_null()) {
            print_error("Towns and terminals are desynced at " + pop->get_location());
            continue;
        }
        town->sell_to_pop(pop);
    }

    CONST_PEASANT_POP_LOOP() {
        BasePop* pop = get_pop(pop_id);
        pop->month_tick();
        if (!closest_town_to_tile.count(pop->get_location())) {
            // Peasant pops may exist in province without town
            continue;
        }
        Ref<Town> town = get_closest_town_to_pop(pop);
        if (town.is_null()) {
            print_error("Towns and terminals are desynced at " + pop->get_location());
            continue;
        }
        town->sell_to_pop(pop);
    }

}