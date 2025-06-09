#include "../singletons/terminal_map.hpp"
#include "province.hpp"
#include "base_pop.hpp"
#include "rural_pop.hpp"
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

    ClassDB::bind_method(D_METHOD("day_tick"), &Province::day_tick);
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
    for (BasePop* pop: pops) {
        memdelete(pop);
    }
    pops.clear();
}

void Province::add_tile(Vector2i coords) {
    tiles.push_back(coords);
}

int Province::get_population() const {
    return population;
}

void Province::add_population(int population_to_add) {
    population += population_to_add;
}

void Province::set_population(int new_population) {
    population = new_population;
}

int Province::get_province_id() const {
    return province_id;
}

int Province::get_country_id() const {
    return country_id;
}

void Province::set_country_id(int p_country_id) {
    country_id = p_country_id;
}

Array Province::get_tiles() const {
    Array a;
    for (Vector2i tile: tiles) {
        a.append(tile);
    }
    return a;
}

//Used to pick a place for random industries, don't pick places with industries
Vector2i Province::get_random_tile() const {
    std::vector<Vector2i> tiles_copy;
    for (Vector2i tile: tiles) {
        if (!terminal_tiles.count(tile)) {
            tiles_copy.push_back(tile);
        }
    }
    if (tiles_copy.size() == 0) {
        return Vector2i(0, 0);
    } else if (tiles_copy.size() == 1) {
        return tiles_copy.at(0);
    }
    return tiles_copy.at(rand() % (tiles_copy.size() - 1));
}

void Province::add_terminal(Vector2i tile) {
    if (terminal_tiles.count(tile) != 0) {
        ERR_FAIL_MSG("Already has a terminal there");
        return;
    }
    terminal_tiles.insert(tile);
}

void Province::remove_terminal(Vector2i tile) { //BUG: Never gets called when deleting terminals
    if (terminal_tiles.count(tile) == 0) {
        ERR_FAIL_MSG("No terminal there");
        return;
    }
    terminal_tiles.erase(tile);
}
Array Province::get_terminal_tiles() const {
    Array a;
    for (const auto tile: terminal_tiles) {
        a.push_back(tile);
    }
    return a;
}

void Province::create_pops() {
    int number_of_rural_pops = floor(population * 0.8 / BasePop::get_people_per_pop());
	int number_of_city_pops = floor(population * 0.2 / BasePop::get_people_per_pop());
	for (int i = 0; i < number_of_rural_pops; i++) {
        pops.push_back(memnew(BasePop(province_id, 0)));
    }
		
	std::vector<Vector2i> towns = get_town_tiles();
	//If no cities, then turn rest of population into rural pops
	if (towns.size() == 0) {
		for (int i = 0; i < number_of_city_pops; i++) {
            pops.push_back(memnew(BasePop(province_id, 0)));
        }
		return;
    }
	int index = 0;
	for (int i = 0; i < number_of_city_pops; i++) {
        Ref<Town> town = TerminalMap::get_instance() -> get_town(towns[index]);
        if (town.is_valid()) {
            town -> add_pop(memnew(BasePop(province_id, 0)));
        }
        index = (index + 1) % towns.size();
    }
}

std::vector<Vector2i> Province::get_town_tiles() const {
    std::vector<Vector2i> toReturn;
    for (const auto &tile: terminal_tiles) {
        if (TerminalMap::get_instance() -> is_town(tile)) {
            toReturn.push_back(tile);
        }
    }
    return toReturn;
}

int Province::count_pops() const {
    return pops.size();
}

Ref<FactoryTemplate> Province::find_employment(BasePop* pop) const {
    float max_wage = 0.0;
    Ref<FactoryTemplate> best_fact = nullptr;
    
    if (dynamic_cast<TownPop*>(pop)) {
        best_fact = find_urban_employment(pop);
    } else if (dynamic_cast<RuralPop*>(pop)) {
        for (const auto tile: terminal_tiles) {
            Ref<FactoryTemplate> fact = Ref<FactoryTemplate>(TerminalMap::get_instance() -> get_terminal(tile));
            if (fact.is_valid() && pop -> will_work_here(fact) && fact -> get_wage() > max_wage) {
                best_fact = fact;
                max_wage = fact -> get_wage();
            }
 
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
        
        TerminalMap::get_instance() -> lock(fact -> get_location());
        if (fact -> get_wage() > max_wage) {
            best_fact = fact;
            max_wage = fact -> get_wage();
        }
        TerminalMap::get_instance() -> unlock(fact -> get_location());
    }
    return best_fact;
}

void Province::day_tick() {

}

void Province::month_tick() {
    for (BasePop* pop: pops) {
        if (pop -> is_seeking_employment()) {
            Ref<FactoryTemplate> work = find_employment(pop);
            if (work.is_valid()) {
                pop -> work_here(work);
            }
        }
    }
}