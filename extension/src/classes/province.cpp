#include "province.hpp"

void Province::_bind_methods() {

}
    

Province* Province::create(int p_prov_id) {
    return memnew(Province(p_prov_id));
}

void Province::initialize(int p_prov_id) {
    province_id = p_prov_id;
    population = 0;
	tiles = {};
	terminal_tiles = {};
}

Province::Province() {
    initialize();
}
Province::Province(int p_prov_id) {
    initialize(p_prov_id);
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

Vector2i Province::get_random_tile() const {
    return tiles.at(rand() % (tiles.size() - 1));
}

void Province::add_terminal(Vector2i tile, Terminal* term) {
    if (terminal_tiles.count(tile) != 0) {
        ERR_FAIL_MSG("Already has a terminal there");
        return;
    }
    terminal_tiles[tile] = term;
}

void Province::remove_terminal(Vector2i tile) { //BUG: Never gets called when deleting terminals
    if (terminal_tiles.count(tile) == 0) {
        ERR_FAIL_MSG("No terminal there");
        return;
    }
    terminal_tiles.erase(tile);
}
Array Province::get_terminals() const {
    Array a;
    for (const auto &[__, term]: terminal_tiles) {
        a.push_back(term);
    }
    return a;
}

void Province::create_pops() {
    int number_of_rural_pops = floor(population * 0.8 / BasePop::get_people_per_pop());
	int number_of_city_pops = floor(population * 0.2 / BasePop::get_people_per_pop());
	for (int i = 0; i < number_of_rural_pops; i++) {
        pops.push_back(memnew(BasePop(province_id, 0)));
    }
		
	std::vector<Town*> towns = get_towns();
	//If no cities, then turn rest of population into rural pops
	if (towns.size() == 0) {
		for (int i = 0; i < number_of_city_pops; i++) {
            pops.push_back(memnew(BasePop(province_id, 0)));
        }
		return;
    }
	int index = 0;
	for (int i = 0; i < number_of_city_pops; i++) {
		Town* town = towns[index];
		town -> add_pop(memnew(BasePop(province_id, 0)));
		index = (index + 1) % towns.size();
    }
}

std::vector<Town*> Province::get_towns() const {
    std::vector<Town*> toReturn;
    for (const auto &[__, term]: terminal_tiles) {
        if (Town* town = dynamic_cast<Town*>(term)) {
            toReturn.push_back(town);
        }
    }
    return toReturn;
}

int Province::count_pops() const {
    return pops.size();
}
FactoryTemplate* Province::find_employment(BasePop* pop) const {
    if (dynamic_cast<TownPop*>(pop)) {
        //TODO: Add buildings to cities
    }
	if (dynamic_cast<RuralPop*>(pop)) {
        for (const auto &[__, term]: terminal_tiles) {
            FactoryTemplate* fact = dynamic_cast<FactoryTemplate*>(term);
            if (fact && will_work_here(pop, fact))
				return fact;
        }
    }
	return nullptr;
}

bool Province::will_work_here(BasePop* pop, FactoryTemplate* fact) const {
    if (!fact -> is_hiring()) return false;
	float income = fact -> get_wage();
	if (pop -> is_income_acceptable(income)) return true;
	return false;
}