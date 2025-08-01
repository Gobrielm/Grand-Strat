#include "town.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Town::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location"), &Town::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location"), &Town::initialize);

    // Trade-related
    ClassDB::bind_method(D_METHOD("is_price_acceptable", "type", "price"), &Town::is_price_acceptable);
    ClassDB::bind_method(D_METHOD("get_desired_cargo", "type", "price"), &Town::get_desired_cargo);

    // Factory and Pop management
    
    ClassDB::bind_method(D_METHOD("add_factory", "factory"), &Town::add_factory);
    ClassDB::bind_method(D_METHOD("get_factories"), &Town::get_factories);

    ClassDB::bind_method(D_METHOD("add_pop", "pop"), &Town::add_pop);
    ClassDB::bind_method(D_METHOD("get_last_month_supply"), &Town::get_last_month_supply);
    ClassDB::bind_method(D_METHOD("get_last_month_demand"), &Town::get_last_month_demand);

    // Fulfillment
    ClassDB::bind_method(D_METHOD("get_fulfillment_dict"), &Town::get_fulfillment_dict);
    ClassDB::bind_method(D_METHOD("get_fulfillment", "type"), &Town::get_fulfillment);
    

    // Selling
    ClassDB::bind_method(D_METHOD("get_total_pops"), &Town::get_total_pops);
    ClassDB::bind_method(D_METHOD("sell_to_other_brokers"), &Town::sell_to_other_brokers);
    ClassDB::bind_method(D_METHOD("distribute_from_order", "order"), &Town::distribute_from_order);

    // Game Loop
    ClassDB::bind_method(D_METHOD("day_tick"), &Town::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &Town::month_tick);
}

Town::Town(): Broker(Vector2i(0, 0), 0) {
    set_max_storage(DEFAULT_MAX_STORAGE);
    local_pricer = memnew(LocalPriceController);
    for (const auto& [type, __]: CargoInfo::get_instance()->get_base_prices()) {
        market_storage[type] = std::multiset<TownCargo*, TownCargo::TownCargoPtrCompare>();
        current_totals[type] = 0;
    }
}

Town::Town(Vector2i new_location): Broker(new_location, 0) {
    set_max_storage(DEFAULT_MAX_STORAGE);
    local_pricer = memnew(LocalPriceController);
    for (const auto& [type, __]: CargoInfo::get_instance()->get_base_prices()) {
        market_storage[type] = std::multiset<TownCargo*, TownCargo::TownCargoPtrCompare>();
        current_totals[type] = 0;
    }
}

Ref<Town> Town::create(Vector2i new_location) {
    return Ref<Town>(memnew(Town(new_location)));
}

Town::~Town() {
    std::scoped_lock lock(m);
    internal_factories.clear();
}

void Town::initialize(Vector2i new_location) {
    Broker::initialize(new_location, 0);
    set_max_storage(DEFAULT_MAX_STORAGE);
    local_pricer = memnew(LocalPriceController);
}

std::vector<int> Town::get_supply() const {
    std::scoped_lock lock(m);
    return local_pricer -> get_supply();
}

std::vector<int> Town::get_demand() const {
    std::scoped_lock lock(m);
   return local_pricer -> get_demand();
}

int Town::get_supply(int type) const {
    std::scoped_lock lock(m);
    return local_pricer -> get_supply(type);
}
int Town::get_demand(int type) const {
    std::scoped_lock lock(m);
    return local_pricer -> get_demand(type);
}

//To Buy
bool Town::is_price_acceptable(int type, float price) const {
    std::scoped_lock lock(m);
    return (local_pricer -> get_local_price(type)) >= price;
}

int Town::get_desired_cargo(int type, float price) const { // BUG/TODO: Kinda outdated
    if (is_price_acceptable(type, price)) {
        std::scoped_lock lock(m);
        int amount_wanted = ceil(local_pricer -> get_last_month_demand(type) / 25); // Buy a bit more than last month just to fill storage
        
		return int(cash / price);
    }
	return 0;
}

int Town::get_cargo_amount(int type) const {
    ERR_FAIL_COND_V_EDMSG(!current_totals.count(type), 0, "No cargo of type: " + String::num(type));
    return current_totals.at(type);
}

// Production
Dictionary Town::get_fulfillment_dict() const {
    Dictionary d;
    for (int type = 0; type < get_supply().size(); type++) {
        d[type] = get_fulfillment(type);
    }
    return d;
}

float Town::get_fulfillment(int type) const {
    std::scoped_lock lock(m);
    int supply = local_pricer->get_supply(type);
    int demand = local_pricer->get_demand(type);
    if (supply == 0)
		return 5;
	return float(demand / supply);
}

void Town::add_factory(Ref<FactoryTemplate> fact) {
    {
        std::scoped_lock lock(m);
        if (!internal_factories.count(fact->get_player_owner()))
            internal_factories[fact->get_player_owner()] = std::vector<Ref<FactoryTemplate>>();
        internal_factories[fact->get_player_owner()].push_back(fact);
    }
	fact->add_connected_broker(this);
}

Array Town::get_factories() const {
    Array a;
    std::scoped_lock lock(m);
    for (const auto &[__, v]: internal_factories){
        for (Ref<FactoryTemplate> f: v) {
            a.push_back(f);
        }
    }   
    return a;
}

Dictionary Town::get_last_month_supply() const {
    Dictionary d = {};
    std::vector<int> v;
    {
        std::scoped_lock lock(m);
        v = local_pricer -> get_last_month_supply();
    }
    for (int type = 0; type < v.size(); type++) {
        d[type] = v[type];
    }
    return d;
}

Dictionary Town::get_last_month_demand() const {
    Dictionary d = {};
    std::vector<int> v;
    {
        std::scoped_lock lock(m);
        v = local_pricer -> get_last_month_demand();
    }
    for (int type = 0; type < v.size(); type++) {
        d[type] = v[type];
    }
    return d;
}

//Pop stuff
void Town::add_pop(int pop_id) {
    std::scoped_lock lock(m);
    ERR_FAIL_COND_MSG(town_pop_ids.count(pop_id), "Pop of id has already been created");
    town_pop_ids.insert(pop_id);
}

void Town::sell_to_pop(BasePop* pop) { // Called from Province, do not call locally
    std::unique_lock lock(m);

    for (auto& [type, ms] : market_storage) {
        local_pricer->add_demand(type, pop->get_desired(type));
        // continue; // TEMP

        if (ms.empty()) {
            continue;
        }
        auto it = ms.begin();
        TownCargo* town_cargo = *it;
        int desired = pop->get_desired(type, town_cargo->price);

        while (desired > 0) {
            int amount = std::min(desired, town_cargo->amount);
            pop->buy_good(type, amount, town_cargo->price);
            current_totals[type] -= amount;

            {
                lock.unlock();
                town_cargo->sell_cargo(amount); // Makes call on other broker, so unlock for safety
                lock.lock();
            }
            

            if (town_cargo->amount == 0 && !ms.empty()) {
                ms.erase(it); // Erases first element
                delete town_cargo;
            }

            if (ms.empty()) break;

            it = ms.begin();
            town_cargo = *it;
            desired = pop->get_desired(type, town_cargo->price);
        }
    }
}

void Town::pay_factory(int amount, float price, Vector2i source) {
    Ref<FactoryTemplate> factory = TerminalMap::get_instance()->get_terminal_as<FactoryTemplate>(source);
    if (factory.is_null()) return; 
    factory->add_cash(amount * price);
}   

int Town::get_total_pops() const {
    std::scoped_lock lock(m);
    return town_pop_ids.size();
}

Ref<FactoryTemplate> Town::find_employment(BasePop* pop) const {
    float max_wage = 0.0;
    Ref<FactoryTemplate> best_fact = nullptr;
    
    for (const auto &[__, fact_vector]: internal_factories) {
        for (const auto &fact: fact_vector) {
            if (fact->is_hiring(pop) && fact -> get_wage() > max_wage)
                best_fact = fact;
                max_wage = fact -> get_wage();
        }
    }
	return best_fact;
}

//Selling to brokers
void Town::sell_to_other_brokers() {
    std::vector<int> supply = get_supply();
	for (int type = 0; type < supply.size(); type++) {
        report_demand_of_brokers(type);
        if (market_storage[type].empty()) continue;
		distribute_type(type);
    }
}

void Town::distribute_type(int type) {
    //Distribute to local factories
	for (const auto &[__, fact_vector]: internal_factories) {
		for (Ref<FactoryTemplate> fact: fact_vector) {
			if (fact->does_accept(type)) {
                distribute_type_to_broker(type, fact);
            }
        }
    }
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
	//Distribute to other brokers
	for (const auto &tile: connected_brokers) {
        s.insert(tile);
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
		if (broker->does_accept(type)) {
            distribute_type_to_broker(type, broker);
        }
    }
    // Distribute using stations to other brokers
    for (const auto& tile: connected_stations) {
        Ref<RoadDepot> road_depot = TerminalMap::get_instance() -> get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_null()) continue;
        std::vector<Ref<Broker>> other_brokers = road_depot->get_available_brokers(type);
        for (auto broker: other_brokers) {
            if (!s.count(broker->get_location())) {
                s.insert(broker->get_location());
                distribute_type_to_broker(type, broker, road_depot);
            }
        }
    }
}

void Town::distribute_type_to_broker(int type, Ref<Broker> broker, Ref<RoadDepot> road_depot) {
    std::scoped_lock lock(m);
    auto& ms = market_storage[type];
    float broker_price = broker->get_local_price(type);
    if (ms.empty()) {
        return;
    }
    TownCargo* town_cargo = *ms.begin(); // town_cargo comes from a fact/broker selling it, they would seek a higher price
    float fee = road_depot.is_valid() ? road_depot->get_fee() : 0; // Between 0 - 1
    if (town_cargo->price > broker_price * (1 + fee)) {
        return;
    }

    int desired = broker -> get_desired_cargo(type, town_cargo->price);
    while (desired > 0) {
        int amount = std::min(desired, town_cargo->amount);
        broker->buy_cargo(town_cargo); // A town will not pay, a broker will atp
        current_totals[type] -= amount;

        town_cargo->transfer_cargo(amount);

        if (road_depot.is_valid()) {
            road_depot->add_cash(road_depot->get_fee());
        }
        
        if (!town_cargo->amount) {
            ms.erase(ms.begin());
            delete town_cargo;
            if (ms.empty()) {
                break;
            }
        }

        town_cargo = *ms.begin();
        desired = broker -> get_desired_cargo(type, town_cargo->price);
    }
}

std::vector<bool> Town::get_accepts_vector() const {
    std::vector<bool> v;
    int size = get_supply().size();
    v.resize(size);
    for (int type = 0; type < size; type++) {
        v[type] = does_accept(type);
    }
    return v;
}

void Town::buy_cargo(int type, int amount, float price, int p_terminal_id) {
    TownCargo* town_cargo = new TownCargo(type, amount, price, p_terminal_id);
    {
        std::scoped_lock lock(m);
        local_pricer -> add_supply(type, amount);
        market_storage[type].insert(town_cargo);
        current_totals[type] += amount;
    }
    
}

void Town::buy_cargo(const TownCargo* cargo) {
    TownCargo* town_cargo = new TownCargo(cargo);
    {
        int type = town_cargo->type;
        int amount = town_cargo->amount;
        std::scoped_lock lock(m);
        local_pricer -> add_supply(type, amount);
        market_storage[type].insert(town_cargo);
        current_totals[type] += amount;
    }
}

int Town::add_cargo(int type, int amount) {
    ERR_FAIL_V_EDMSG(0, "Add cargo shouldn't be used on towns");
    return 0; // Need to pay peasant or landowner or something
}

void Town::age_all_cargo() {
    std::scoped_lock lock(m);
    for (auto& [__, ms]: market_storage) {
        for (auto it = ms.begin(); it != ms.end();) {
            if ((++((*it)->age)) > 5) {
                TownCargo* cargo = (*it);
                current_totals[cargo->type] -= cargo->amount;
                cargo->return_cargo(); //Return cargo to broker
                delete cargo;          //Delete old cargo
                it = ms.erase(it);     //Erase from set, and update iterator
            } else {
                ++it;                  //Increment if not deleting    
            }
        }
    }
}

//Economy Stats

void Town::update_buy_orders() {

    std::vector<int> v;
    {
        std::scoped_lock lock(m);
        v = local_pricer -> get_last_month_demand();
    }
    
    for (int type = 0; type < v.size(); type++) {
        if (v[type] == 0) {
            remove_order(type);
            remove_accept(type);
        } else {
            float price = get_local_price(type);
            edit_order(type, v[type], true, price);
            add_accept(type);
        }
    }
}

// Process Hooks
void Town::day_tick() {
    // sell_to_other_brokers();
    // sell_to_other_towns();
}

void Town::month_tick() {
    update_buy_orders();
    {
        std::scoped_lock lock(m);
        local_pricer -> adjust_prices();
    }
    // age_all_cargo();
}
