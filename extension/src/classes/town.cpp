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
    ClassDB::bind_method(D_METHOD("sell_to_pops"), &Town::sell_to_pops);
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
        market_storage[type] = std::priority_queue<TownCargo*, std::vector<TownCargo*>, TownCargo::TownCargoPtrCompare>();
    }
}

Town::Town(Vector2i new_location): Broker(new_location, 0) {
    set_max_storage(DEFAULT_MAX_STORAGE);
    local_pricer = memnew(LocalPriceController);
    for (const auto& [type, __]: CargoInfo::get_instance()->get_base_prices()) {
        market_storage[type] = std::priority_queue<TownCargo*, std::vector<TownCargo*>, TownCargo::TownCargoPtrCompare>();
    }
}

Ref<Town> Town::create(Vector2i new_location) {
    return Ref<Town>(memnew(Town(new_location)));
}

Town::~Town() {
    for (const auto &[__, pop]: city_pops) {
        memdelete(pop);
    }
    city_pops.clear();
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

int Town::get_desired_cargo(int type, float price) const {
    if (is_price_acceptable(type, price)) {
		int amount_could_get = std::min(get_max_storage() - get_cargo_amount(type), get_amount_can_buy(price)); // TODO, add tracker for this since other thing is vector
        int amount_wanted = 0;
        {
            std::scoped_lock lock(m);
            amount_wanted = ceil(local_pricer -> get_last_month_demand(type) / 25); // Buy a bit more than last month just to fill storage
        }
		return std::min(amount_wanted, amount_could_get); 
    }
	return 0;
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
    m.lock();
    if (!internal_factories.count(fact->get_player_owner()))
		internal_factories[fact->get_player_owner()] = std::vector<Ref<FactoryTemplate>>();
	internal_factories[fact->get_player_owner()].push_back(fact);
    m.unlock();
	fact->add_connected_broker(this);
}

Array Town::get_factories() const {
    Array a;
    m.lock();
    for (const auto &[__, v]: internal_factories){
        for (Ref<FactoryTemplate> f: v) {
            a.push_back(f);
        }
    }   
    m.unlock();
    return a;
}

Dictionary Town::get_last_month_supply() const {
    Dictionary d = {};
    m.lock();
    const auto v = local_pricer -> get_last_month_supply();
    m.unlock();
    for (int type = 0; type < v.size(); type++) {
        d[type] = v[type];
    }
    return d;
}

Dictionary Town::get_last_month_demand() const {
    Dictionary d = {};
    m.lock();
    const auto v = local_pricer -> get_last_month_demand();
    m.unlock();
    for (int type = 0; type < v.size(); type++) {
        d[type] = v[type];
    }
    return d;
}

//Pop stuff
void Town::add_pop(BasePop* pop) {
    std::scoped_lock lock(m);
    ERR_FAIL_COND_MSG(city_pops.count(pop -> get_pop_id()) != 0, "Pop of id has already been created");
    city_pops[pop -> get_pop_id()] = pop;
}

void Town::sell_to_pops() {
    //market -> get_supply().size() represents all goods
    sell_to_city_pops();
    sell_to_rural_pops();
    
}

void Town::sell_to_city_pops() {
    for (const auto &[__, pop]: city_pops) {
        sell_to_pop(pop);
    }
}

void Town::sell_to_rural_pops() {
    for (const auto &[__, pop]: get_rural_pops()) {
        sell_to_pop(pop);
    }
}

void Town::sell_to_pop(BasePop* pop) {
    pop->month_tick();
    
    for (auto& [type, pq]: market_storage) {
        {
            std::scoped_lock lock(m);
            local_pricer -> add_demand(type, pop -> get_desired(type));
        }
        if (pq.empty()) {
            continue;
        }
        TownCargo* town_cargo = pq.top();
        int desired = pop -> get_desired(type, town_cargo->price);
        while (desired > 0) {
            int amount = std::min(desired, town_cargo->amount);
            pop->buy_good(type, amount, town_cargo->price);
            town_cargo->sell_cargo(amount);

            if (!town_cargo->amount) {
                pq.pop();
                if (pq.empty()) {
                    break;
                }
                delete town_cargo;
                town_cargo = pq.top();
            }
            desired = pop -> get_desired(type, town_cargo->price);
        }
    }
}

void Town::pay_factory(int amount, float price, Vector2i source) {
    Ref<FactoryTemplate> factory = TerminalMap::get_instance()->get_terminal_as<FactoryTemplate>(source);
    if (factory.is_null()) return; 
    factory->add_cash(amount * price);
}   

std::unordered_map<int, BasePop*> Town::get_rural_pops() const {
    Ref<ProvinceManager> province_manager =  ProvinceManager::get_instance();
    Province* province = province_manager->get_province(province_manager->get_province_id(get_location()));// Province where city is located
    return province->get_rural_pops();
}

int Town::get_total_pops() const {
    std::scoped_lock lock(m);
    return city_pops.size();
}

Ref<FactoryTemplate> Town::find_employment(BasePop* pop) const {
    float max_wage = 0.0;
    Ref<FactoryTemplate> best_fact = nullptr;
    
    for (const auto &[__, fact_vector]: internal_factories) {
        for (const auto &fact: fact_vector) {
            if (pop -> will_work_here(fact) && fact -> get_wage() > max_wage)
                best_fact = fact;
                max_wage = fact -> get_wage();
        }
    }
	return best_fact;
}

int Town::get_number_of_broke_pops() const {
    int count = 0;
    for (const auto& [__, pop]: city_pops) {
        if (pop -> get_wealth() < 20) {
            count++;
        }
    }
    for (const auto& [__, pop]: get_rural_pops()) {
        if (pop -> get_wealth() < 20) {
            count++;
        }
    }
    return count;
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
                distribute_type_to_broker(type, fact); //Use ptr becuase it isn't in terminal_map
            }
        }
    }
	//Distribute to stations, ports, or other brokers
	for (const auto &tile: connected_brokers) {
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
		if (broker->does_accept(type)) {
            distribute_type_to_broker(type, broker);
        }
    }
}

void Town::distribute_type_to_broker(int type, Ref<Broker> broker) {
    auto& pq = market_storage[type];
    float broker_price = broker->get_local_price(type);
    if (pq.empty()) {
        return;
    }
    TownCargo* town_cargo = pq.top(); // town_cargo comes from a fact/broker selling it, they would seek a higher price
    int desired = broker -> get_desired_cargo(type, town_cargo->price);
    while (desired > 0) {
        int amount = std::min(desired, town_cargo->amount);
        broker->buy_cargo(type, amount, town_cargo->price, town_cargo->source);
        Ref<Town> town = broker;
        if (!town.is_valid()) { // If it is a town then just transfer cargo, not sold yet
            town_cargo->sell_cargo(amount);
        } else {
            town_cargo->transfer_cargo(amount);
        }
        
        if (!town_cargo->amount) {
            pq.pop();
            if (pq.empty()) {
                break;
            }
            delete town_cargo;
        }

        town_cargo = pq.top();
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

void Town::buy_cargo(int type, int amount, float price, Vector2i seller) {
    TownCargo* town_cargo = new TownCargo(type, amount, price, seller);
    {
        std::scoped_lock lock(m);
        local_pricer -> add_supply(type, amount);
    }
    market_storage[town_cargo->type].push(town_cargo);
}

int Town::add_cargo(int type, int amount) {
    return 0; // Need to pay peasant or landowner or something
}

//Economy Stats

float Town::get_total_wealth_of_pops() {
    float total = 0.0;
    for (const auto &[__, pop]: city_pops) {
        total += pop -> get_wealth();
    }
    return total;
}

float Town::get_needs_met_of_pops() {
    float total = 0.0;
    for (const auto &[__, pop]: city_pops) {
        total += pop -> get_average_fulfillment();
    }
    return total;
}

void Town::update_buy_orders() {
    const auto v = local_pricer -> get_last_month_demand();
    for (int type = 0; type < v.size(); type++) {
        if (v[type] == 0) {
            remove_order(type);
            remove_accept(type);
        } else {
            edit_order(type, v[type], true, get_local_price(type));
            add_accept(type);
        }
    }
}

// Process Hooks
void Town::day_tick() {
    // sell_to_other_brokers(); Doesn't work anymore, need to redo, should include factories, construction sites, and warehouses
    // sell_to_other_towns();
}

void Town::month_tick() {
    sell_to_pops();
    update_buy_orders();
    m.lock();
    local_pricer -> adjust_prices();
    m.unlock();
    set_max_storage(city_pops.size() * 5); // Update size according to number of pops
}
