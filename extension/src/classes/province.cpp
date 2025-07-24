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
    for (const auto &[__, pop]: rural_pops) {
        memdelete(pop);
    }
    for (const auto &[__, pop]: peasant_pops) {
        memdelete(pop);
    }
    rural_pops.clear();
}

void Province::add_tile(Vector2i coords) {
    tiles.push_back(coords);
}

int Province::get_population() const {
    return population;
}

int Province::get_number_of_city_pops() const {
    return number_of_city_pops;
}

const std::unordered_map<int, BasePop*>& Province::get_rural_pops() const {
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

const std::vector<Vector2i>& Province::get_tiles_vector() const {
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
    std::scoped_lock lock(m);
    if (terminal_tiles.count(tile) != 0) {
        ERR_FAIL_MSG("Already has a terminal there");
        return;
    }
    terminal_tiles.insert(tile);
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

void Province::create_pops() {
    int number_of_peasant_pops = floor(population * 0.6 / BasePop::get_people_per_pop());
    int number_of_rural_pops = floor(population * 0.2 / RuralPop::get_people_per_pop());
	number_of_city_pops = floor(population * 0.2 / TownPop::get_people_per_pop());
    for (int i = 0; i < number_of_peasant_pops; i++) {
        create_peasant_pop(0);
    }
	for (int i = 0; i < number_of_rural_pops; i++) {
        create_rural_pop(0);
    }
		
	std::vector<Vector2i> towns = get_town_tiles();
	//If no cities, then turn rest of population into peasant pops
	if (towns.size() == 0) {
        m.lock();
		for (int i = 0; i < number_of_city_pops; i++) {
            create_peasant_pop(0);
        }
        number_of_city_pops = 0;
        m.unlock();
		return;
    }
	int index = 0;
	for (int i = 0; i < number_of_city_pops; i++) {
        Ref<Town> town = TerminalMap::get_instance() -> get_town(towns[index]);
        
        if (town.is_valid()) {
            m.lock();
            town -> add_pop(memnew(BasePop(province_id, 0)));
            m.unlock();
        }
        
        index = (index + 1) % towns.size();
    }
}

void Province::create_peasant_pop(Variant culture) {
    PeasantPop* pop = memnew(PeasantPop(province_id, culture));
    {
        std::scoped_lock lock(m);
        peasant_pops[pop->get_pop_id()] = pop;
    }
}

void Province::create_rural_pop(Variant culture) {
    RuralPop* pop = memnew(RuralPop(province_id, culture));
    {
        std::scoped_lock lock(m);
        rural_pops[pop->get_pop_id()] = pop;
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
    std::scoped_lock lock(m);
    return rural_pops.size() + peasant_pops.size();
}

void Province::find_employment_for_pops() {
    Ref<FactoryTemplate> work;

    for (const auto [__, pop]: rural_pops) {
        if (pop -> is_seeking_employment()) {
            if (work.is_valid() && pop->will_work_here(work)) {
                pop -> work_here(work);
            } else {
                work = find_employment(pop);
                if (work.is_valid()) {
                    pop -> work_here(work);
                }
            }
        }
    }
}

Ref<FactoryTemplate> Province::find_employment(BasePop* pop) const {
    float max_wage = 0.0;
    Ref<FactoryTemplate> best_fact = Ref<FactoryTemplate>(nullptr);
    
    for (const auto tile: terminal_tiles) {
        Ref<FactoryTemplate> fact = TerminalMap::get_instance() -> get_terminal_as<FactoryTemplate>(tile);
        if (fact.is_valid() && pop -> will_work_here(fact) && fact -> get_wage() > max_wage) {
            best_fact = fact;
            max_wage = fact -> get_wage();
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

void Province::peasant_tick() { // Creates grain for themselves a tiny bit to give to the towns
    double extra_grain = 0;
    for (const auto& [__, pop]: peasant_pops) {
        extra_grain += (rand() % 3 + 1) / 10; // Creates between 0.1 - 0.4, ie can fed 1/10 to 4/10 other people plus themselves
    }
    std::vector<Vector2i> town_tiles = get_town_tiles();
    int for_each = extra_grain / town_tiles.size();
    for (const Vector2i &tile: town_tiles) {
        Ref<Town> town = TerminalMap::get_instance() -> get_town(tile);
        town->add_cargo(CargoInfo::get_instance()->get_cargo_type("grain"), for_each);
    }
}

void Province::month_tick() {
    find_employment_for_pops();
    // Employ or find education for peasants
    peasant_tick();
}