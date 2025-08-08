#include "town.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <map>
#include <algorithm>
#include <random>

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
        cargo_sell_orders[type] = std::multiset<TownCargo*, TownCargo::TownCargoPtrCompare>();
        current_totals[type] = 0;
        current_prices[type] = 0;
    }
}

Town::Town(Vector2i new_location): Broker(new_location, 0) {
    set_max_storage(DEFAULT_MAX_STORAGE);
    local_pricer = memnew(LocalPriceController);
    for (const auto& [type, __]: CargoInfo::get_instance()->get_base_prices()) {
        cargo_sell_orders[type] = std::multiset<TownCargo*, TownCargo::TownCargoPtrCompare>();
        current_totals[type] = 0;
        current_prices[type] = 0;
    }
}

Ref<Town> Town::create(Vector2i new_location) {
    return Ref<Town>(memnew(Town(new_location)));
}

Town::~Town() {
    std::scoped_lock lock(internal_factories_mutex);
    internal_factories.clear();
}

void Town::initialize(Vector2i new_location) {
    Broker::initialize(new_location, 0);
    set_max_storage(DEFAULT_MAX_STORAGE);
    local_pricer = memnew(LocalPriceController);
}

std::vector<float> Town::get_supply() const {
    std::scoped_lock lock(m);
    return local_pricer -> get_supply();
}

std::vector<float> Town::get_demand() const {
    std::scoped_lock lock(m);
   return local_pricer -> get_demand();
}

float Town::get_supply(int type) const {
    std::scoped_lock lock(m);
    return local_pricer -> get_supply(type);
}
float Town::get_demand(int type) const {
    std::scoped_lock lock(m);
    return local_pricer -> get_demand(type);
}

//To Buy
bool Town::is_price_acceptable(int type, float price) const {
    return (get_local_price(type) / price) > 0.8;
}

int Town::get_desired_cargo(int type, float price) const { // BUG/TODO: Kinda outdated
    return std::numeric_limits<int>::max();
}

int Town::get_desired_cargo_unsafe(int type, float price) const {
    return get_desired_cargo(type, price);
}

float Town::get_cargo_amount(int type) const {
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
        std::scoped_lock lock(internal_factories_mutex);
        internal_factories[fact->get_player_owner()].push_back(fact);
    }
	fact->add_connected_broker(this);
}

Array Town::get_factories() const {
    Array a;
    std::scoped_lock lock(internal_factories_mutex);
    for (const auto &[__, v]: internal_factories){
        for (Ref<FactoryTemplate> f: v) {
            a.push_back(f);
        }
    }   
    return a;
}

Dictionary Town::get_last_month_supply() const {
    Dictionary d = {};
    std::vector<float> v;
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
    std::vector<float> v;
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
    std::unordered_map<int, float> money_to_pay; // Queued up money to distribute to owners of cargo sold
    std::scoped_lock lock(m);

    for (auto& [type, ms] : cargo_sell_orders) {
        auto sell_it = ms.begin();
        TownCargo* sell_order;
        int desired = pop->get_desired(type);
        if (desired == 0) {
            continue;
        }
        local_pricer->add_demand(type, desired);

        while (sell_it != ms.end()) {
            sell_order = *sell_it;

            const float buyer_price = pop->get_buy_price(type, get_local_price_unsafe(type));
            buy_orders_price_map[type][round(buyer_price * 10)] += desired;
            const float seller_price = sell_order->price;
            float price = (buyer_price + seller_price) / 2; // Price average

            if (((price / buyer_price) - 1) > PopOrder::MAX_DIFF) { // Too high for buyer no deal
                break;
            }
            if ((1 - (price / seller_price)) > PopOrder::MAX_DIFF) { // Too low for seller no deal
                break;
            }

            int amount = std::min(pop->get_desired(type, price), sell_order->amount);
            if (amount == 0) {
                break;
            }

            sell_order->sell_cargo(amount, price, money_to_pay); // Calls with money_to_pay
            pop->buy_good(type, amount, price);

            if (sell_order->amount == 0) {
                sell_it = ms.erase(sell_it);
                delete_town_cargo(sell_order);
            } else {
                break;
            }
        }
    }

    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    for (const auto &[terminal_id, to_pay]: money_to_pay) {
        Ref<Broker> broker = terminal_map->get_terminal_as<Broker>(terminal_id);
        if (broker.is_null()) continue;
        
        broker->add_cash(to_pay);
    }
}

void Town::delete_town_cargo(TownCargo *sell_order) {
    town_cargo_tracker[sell_order->terminal_id].erase(sell_order->type);
    delete sell_order;
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

    std::scoped_lock lock(internal_factories_mutex);
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
    std::vector<float> supply = get_supply();
	for (int type = 0; type < supply.size(); type++) {
        report_demand_of_brokers(type);
        if (cargo_sell_orders[type].empty()) continue;
		distribute_type(type);
    }
}

void Town::distribute_type(int type) {
    //Distribute to local factories
    {
        std::scoped_lock lock(internal_factories_mutex);
        for (const auto& [__, fact_vector]: internal_factories) {
            for (Ref<FactoryTemplate> fact: fact_vector) {
                if (fact->does_accept(type)) {
                    distribute_type_to_broker(type, fact);
                }
            }
        }
    }
	
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    s.insert(get_location());
	//Distribute to other brokers
	for (const auto tile: connected_brokers) {
        s.insert(tile);
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
		if (broker->does_accept(type)) {
            distribute_type_to_broker(type, broker);
        }
    }

    // Distribute using stations to other brokers
    for (const auto tile: connected_stations) {
        Ref<RoadDepot> road_depot = TerminalMap::get_instance() -> get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_null()) continue;
        std::vector<Ref<Broker>> other_brokers = road_depot->get_available_brokers(type);
        for (auto& broker: other_brokers) {
            if (!s.count(broker->get_location())) {
                s.insert(broker->get_location());
                distribute_type_to_broker(type, broker, road_depot);
            }
        }
    }
}


void Town::distribute_type_to_broker(int type, Ref<Broker> broker, Ref<RoadDepot> road_depot) {
    std::vector<TownCargo*> cargo_to_send; // Queued up Town Cargo to sell to brokers
    bool is_depot_valid = road_depot.is_valid();
    float fee = 0.0;
    if (is_depot_valid) {
        std::scoped_lock lock2(road_depot->m); // Lock Depot
        fee = road_depot->get_fee(); // Between 0 - 1
    }

    {
        std::scoped_lock lock1(broker->m); // Lock Broker
        std::scoped_lock lock(m); //  Lock self
    
        auto& ms = cargo_sell_orders[type];
        if (ms.empty()) return;

        auto it = ms.begin();
        TownCargo* town_cargo = *it; // town_cargo comes from a fact/broker selling it, they would seek a higher price

        float buyer_price = broker->get_local_price_unsafe(type);
        float seller_price = town_cargo->price;
        float price = (buyer_price + seller_price) / 2; // Price average
        int desired = broker -> get_desired_cargo_unsafe(type, price);

        while (desired > 0) {
            

            int amount = std::min(desired, town_cargo->amount);
            if (!broker->can_afford_unsafe(amount * price)) break; // If cannot afford then break out
            TownCargo* town_cargo_copy = new TownCargo(*town_cargo);
            if (is_depot_valid) 
                town_cargo_copy->add_fee_to_pay(road_depot->get_terminal_id(), fee);
            cargo_to_send.push_back(town_cargo_copy);

            desired -= amount;
            current_totals[type] -= amount;
            town_cargo->transfer_cargo(amount);
            
            if (town_cargo->amount == 0) {
                it = ms.erase(it);
                delete_town_cargo(town_cargo);
                if (ms.empty()) {
                    break;
                }
                town_cargo = *it;
            }
            seller_price = town_cargo->price;
            price = (buyer_price + seller_price) / 2; // Price average
        }
    }
    // No locks at this point
    for (TownCargo* cargo: cargo_to_send) {

        broker->buy_cargo(cargo); // Broker does work around paying fees and owner
        delete cargo;
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

float Town::get_local_price(int type) const {
    
    std::scoped_lock lock(m);
    if (current_prices.at(type) > 0) {
        return current_prices.at(type);
    }
    return local_pricer -> get_local_price(type);
}

float Town::get_local_price_unsafe(int type) const {
    if (current_prices.at(type) > 0) {
        return current_prices.at(type);
    }
    return local_pricer -> get_local_price(type);
}

void Town::buy_cargo(int type, int amount, float price, int p_terminal_id) {
    TownCargo* new_town_cargo = new TownCargo(type, amount, price, p_terminal_id);
    {
        std::scoped_lock lock(m);
        if (does_cargo_exist(p_terminal_id, type)) {
            TownCargo* existing_town_cargo = town_cargo_tracker[p_terminal_id][type];
            encode_existing_cargo(existing_town_cargo, new_town_cargo);
            delete new_town_cargo;
        } else {
            encode_cargo(new_town_cargo);
        }
    }
}

void Town::buy_cargo(const TownCargo* cargo) {
    TownCargo* new_town_cargo = new TownCargo(cargo);
    int p_terminal_id = cargo->terminal_id;
    int type = cargo->type;
    {
        std::scoped_lock lock(m);
        if (does_cargo_exist(p_terminal_id, type)) {
            TownCargo* existing_town_cargo = town_cargo_tracker[p_terminal_id][type];
            encode_existing_cargo(existing_town_cargo, new_town_cargo);
            delete new_town_cargo;
        } else {
            encode_cargo(new_town_cargo);
        }

    }
    
}

void Town::encode_cargo(TownCargo* town_cargo) {
    int type = town_cargo->type;
    int amount = town_cargo->amount;

    local_pricer -> add_supply(type, amount);
    cargo_sell_orders[type].insert(town_cargo);
    current_totals[type] += amount;
}

void Town::encode_existing_cargo(TownCargo* existing_town_cargo, const TownCargo* new_town_cargo) {
    int type = existing_town_cargo->type;
    existing_town_cargo->price = new_town_cargo->price;
    existing_town_cargo->age = 0;
    int additional_amount = new_town_cargo->amount;

    local_pricer -> add_supply(type, additional_amount);
    current_totals[type] += additional_amount;
    existing_town_cargo->amount += additional_amount;
}

float Town::add_cargo(int type, float amount) {
    ERR_FAIL_V_EDMSG(0, "Add cargo shouldn't be used on towns");
    return 0;
}

void Town::age_all_cargo() {
    std::unordered_map<int, std::unordered_map<int, int>> cargo_to_return; // Stores money to pay other brokers locally to be done after unlocking
    {
        std::scoped_lock lock(m);
        for (auto& [__, ms]: cargo_sell_orders) {
            for (auto it = ms.begin(); it != ms.end();) { // No iterator
                it = return_cargo(it, cargo_to_return);
            }
        }
    }   
    
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    for (const auto& [terminal_id, m]: cargo_to_return) {
        Ref<FactoryTemplate> factory = terminal_map->get_terminal_as<FactoryTemplate>(terminal_id);
        if (factory.is_null()) continue;; 
        for (const auto& [type, amount]: m) {
            factory->add_cargo(type, amount);
        }
        
    }
}

std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare>::iterator Town::return_cargo(std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare>::iterator cargo_it, std::unordered_map<int, std::unordered_map<int, int>>& cargo_to_return) {
    TownCargo* cargo = (*cargo_it);
    if ((++(cargo->age)) > 5) {
        int type = cargo->type;
        current_totals[type] -= cargo->amount;
        cargo->return_cargo(cargo_to_return); //Return cargo to broker
        cargo_it = cargo_sell_orders[type].erase(cargo_it);     //Erase from set, and update iterator
        delete_town_cargo(cargo);             //Delete old cargo
    } else {
        ++cargo_it;                  //Increment if not deleting    
    }
    return cargo_it;
}

bool Town::does_cargo_exist(int terminal_id, int type) const {
    auto it = town_cargo_tracker.find(terminal_id);
    if (it != town_cargo_tracker.end()) {
        auto type_it = (it->second).find(type);
        return type_it != (it->second).end();
    } 
    return false;
}

//Economy Stats

void Town::update_buy_orders() {

    std::vector<float> v;
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

void Town::update_local_prices() {
    for (const auto &[type, __]: cargo_sell_orders) {
        update_local_price(type);
    }
}

void Town::update_local_price(int type) {
    std::unordered_map<int, int> buy_prices; // Multiples and rounds price by 10
    std::unordered_map<int, int> sell_prices; // Multiples and rounds price by 10
    std::scoped_lock lock(m);
    for (const auto& cargo: cargo_sell_orders[type]) {
        int ten_price = (round(cargo->price * 10.0)); // Rounds to nearest tenth
        buy_prices[ten_price] += cargo->amount;
    }
    for (const auto& [ten_price, amount]: buy_orders_price_map[type]) {
        sell_prices[ten_price] += amount;
    }
    int diff = -1;
    for (const auto& [ten_price, amount]: buy_prices) {
        int temp_diff = abs(sell_prices[ten_price] - amount);
        float price = ten_price / 10.0;
        if (temp_diff < diff) {
            diff = temp_diff;
            current_prices[type] = price;
        }
    }
}

// Process Hooks
void Town::day_tick() {
    sell_to_other_brokers();
    // sell_to_other_towns();
}

void Town::month_tick() {

    update_local_prices();
    update_buy_orders();
    {
        std::scoped_lock lock(m);
        local_pricer -> adjust_supply_demand();
    }
    age_all_cargo();
}
