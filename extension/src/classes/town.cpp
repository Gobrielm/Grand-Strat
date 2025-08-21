#include "town.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"
#include "broker_utility/trade_interaction.hpp"
#include "town_utility/town_local_price_controller.hpp"
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

    // Fulfillment
    ClassDB::bind_method(D_METHOD("get_fulfillment_dict"), &Town::get_fulfillment_dict);
    ClassDB::bind_method(D_METHOD("get_fulfillment", "type"), &Town::get_fulfillment);
    

    // Selling
    ClassDB::bind_method(D_METHOD("get_total_pops"), &Town::get_total_pops);

    // Game Loop
    ClassDB::bind_method(D_METHOD("day_tick"), &Town::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &Town::month_tick);
}

Town::Town(): Broker(Vector2i(0, 0), 0) {
    set_max_storage(DEFAULT_MAX_STORAGE);
    local_pricer = new TownLocalPriceController;
}

Town::Town(Vector2i new_location): Broker(new_location, 0) {
    set_max_storage(DEFAULT_MAX_STORAGE);
    local_pricer = new TownLocalPriceController;
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
    local_pricer = new TownLocalPriceController;
}

TownLocalPriceController* Town::get_local_pricer() const {
    return static_cast<TownLocalPriceController*>(local_pricer);
}

std::unordered_map<int, float> Town::get_supply() const {
    std::scoped_lock lock(m);
    return get_local_pricer() -> get_supply();
}

std::unordered_map<int, float> Town::get_demand() const {
    std::scoped_lock lock(m);
   return get_local_pricer() -> get_demand();
}

float Town::get_supply(int type) const {
    std::scoped_lock lock(m);
    return get_local_pricer() -> get_supply(type);
}
float Town::get_demand(int type) const {
    std::scoped_lock lock(m);
    return get_local_pricer() -> get_demand(type);
}

//To Buy
bool Town::is_price_acceptable(int type, float price) const { // Free Market
    return true;
}

int Town::get_desired_cargo(int type, float price) const { // BUG/TODO: Kinda outdated
    return get_demand(type);
}

int Town::get_desired_cargo_unsafe(int type, float price) const {
    return get_local_pricer() -> get_demand(type);
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
    int supply = get_local_pricer()->get_supply(type);
    int demand = get_local_pricer()->get_demand(type);
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

//Pop stuff
void Town::add_pop(int pop_id) {
    std::scoped_lock lock(m);
    ERR_FAIL_COND_MSG(town_pop_ids.count(pop_id), "Pop of id has already been created");
    town_pop_ids.insert(pop_id);
}

void Town::sell_to_pop(BasePop* pop) { // Called from Province, do not call locally
    std::unordered_map<int, float> money_to_pay; // Queued up money to distribute to owners of cargo sold
    
    {   
        std::scoped_lock lock(m);
        for (auto& [type, ms] : get_local_pricer()->cargo_sell_orders) {
            int desired = pop->get_desired(type);
            float pop_price = pop->get_buy_price(type, get_local_price_unsafe(type));

            if (desired == 0) {
                continue;
            }
            
            auto sell_it = ms.begin();
            std::shared_ptr<TownCargo> sell_order;
            get_local_pricer() -> add_demand(type, pop_price, desired); // Only add demand since its not part of survey_broad_market()
            get_local_pricer() -> add_local_demand(type, desired);

            while (sell_it != ms.end()) {
                ERR_FAIL_COND_MSG((*sell_it).expired(), "EXPIRED POINTER FROM TOWN LOCAL PRICER");
                sell_order = (*sell_it).lock();

                const float seller_price = sell_order->price;
                const float buyer_price = pop->get_buy_price(type, seller_price);

                if (std::isnan(seller_price)) {
                    ERR_FAIL_MSG("seller price is nan");
                }
                if (std::isnan(buyer_price)) {
                    ERR_FAIL_MSG("buyer_price is nan");
                }
                float price = (buyer_price + seller_price) / 2; // Price average

                if (((price / buyer_price) - 1) > PopOrder::MAX_DIFF) { // Too high for buyer no deal
                    break;
                }
                if ((1 - (price / seller_price)) > PopOrder::MAX_DIFF) { // Too low for seller no deal
                    break;
                }

                int amount = std::min(pop->get_desired(type, price), sell_order->amount);
                //AMount is negitive at some pt
                if (amount == 0) {
                    break;
                }
                get_local_pricer()->report_sale(type, price, amount);
                sell_order->sell_cargo(amount, price, money_to_pay); // Calls with money_to_pay
                pop->buy_good(type, amount, price);

                if (sell_order->amount == 0) {
                    sell_it = get_local_pricer()->delete_town_cargo(sell_it);
                } else {
                    break;
                }
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

void Town::pay_factory(int amount, float price, Vector2i source) {
    Ref<FactoryTemplate> factory = TerminalMap::get_instance()->get_terminal_as<FactoryTemplate>(source);
    if (factory.is_null()) return; 
    factory->add_cash(amount * price);
}   

int Town::get_total_pops() const {
    std::scoped_lock lock(m);
    return town_pop_ids.size();
}

std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare> Town::get_employment_sorted_by_wage(PopTypes pop_type) const {
    std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare> s;

    std::scoped_lock lock(internal_factories_mutex);
    for (const auto &[__, fact_vector]: internal_factories) {
        for (const auto &fact: fact_vector) {
            if (fact->is_hiring(pop_type))
                s.insert(fact);
        }
    }
	return s;
}

//Selling to brokers
void Town::distribute_cargo() {
    std::unordered_map<int, float> supply = get_supply();
	for (const auto& [type, __]: supply) {
        survey_broad_market(type);
        if (get_local_pricer()->cargo_sell_orders[type].empty()) continue;
		distribute_type(type);
    }
}

void Town::distribute_type(int type) {
    std::set<TradeInteraction*, TradeInteractionPtrCompare> brokers_to_sell_to = get_brokers_to_distribute_to(type); // Sorted by highest price

    int current_amount = get_cargo_amount(type);
    for (auto it = brokers_to_sell_to.begin(); it != brokers_to_sell_to.end(); it++) {
        TradeInteraction* top = *it;
        if (current_amount > 0) {
            distribute_type_to_broker(type, top->main_buyer, top->middleman);
        }
        delete top;
        current_amount = get_cargo_amount(type);
    }
}

std::set<TradeInteraction*, TradeInteractionPtrCompare> Town::get_brokers_to_distribute_to(int type) {
    std::set<TradeInteraction*, TradeInteractionPtrCompare> brokers_to_sell_to; // Sorted by highest price
    std::unordered_set<int> s; // Broker locations already looked at
    //Distribute to local factories
    {
        std::scoped_lock lock(internal_factories_mutex);
        for (const auto& [__, fact_vector]: internal_factories) {
            for (Ref<FactoryTemplate> fact: fact_vector) {
                add_broker_to_sorted_set(type, s, brokers_to_sell_to, new TradeInteraction(get_price_average(type, fact), fact));
            }
        }
    }
	
    s.insert(terminal_id); // Add self, after internal factories
    
	//Distribute to other brokers
	for (const auto tile: connected_brokers) {
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
        add_broker_to_sorted_set(type, s, brokers_to_sell_to, new TradeInteraction(get_price_average(type, broker), broker));
    }

    // Distribute using stations to other brokers
    for (const auto tile: connected_stations) {
        Ref<RoadDepot> road_depot = TerminalMap::get_instance() -> get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_null()) continue;
        std::vector<Ref<Broker>> other_brokers = road_depot->get_available_brokers(type);
        for (auto& broker: other_brokers) {
            add_broker_to_sorted_set(type, s, brokers_to_sell_to, new TradeInteraction(get_price_average(type, broker), broker, road_depot));
        }
    }
    return brokers_to_sell_to;
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
    
        auto& ms = get_local_pricer()->cargo_sell_orders[type];
        if (ms.empty()) return;

        auto it = ms.begin();
        ERR_FAIL_COND_MSG((*it).expired(), "EXPIRED POINTER FROM TOWN LOCAL PRICER");
        std::shared_ptr<TownCargo> town_cargo = (*it).lock(); // town_cargo comes from a fact/broker selling it, they would seek a higher price

        float buyer_price = broker->get_local_price_unsafe(type);
        float seller_price = town_cargo->price;

        if ((buyer_price / (1 + fee)) > seller_price) return; // If seller wants more than buyer is willing to pay taking into account fee, then simply don't sell

        float price = buyer_price; // Just use buyer price, why not
        int desired = broker -> get_desired_cargo_unsafe(type, price);
        if (Ref<Town>(broker).is_valid()) {
            desired = std::max(broker->get_diff_between_demand_and_supply_unsafe(type) / 30, 0.0f); // TODO: Doesn't work entirely as intended
        }
        

        while (desired > 0) {

            int amount = std::min(desired, town_cargo->amount);
            if (amount <= 0 || !broker->can_afford_unsafe(amount * price)) break; // If cannot afford then break out
            TownCargo* town_cargo_copy = new TownCargo(*town_cargo);
            town_cargo_copy->price = price; // Set price for outgoing cargo
            if (is_depot_valid) 
                town_cargo_copy->add_fee_to_pay(road_depot->get_terminal_id(), fee);
            cargo_to_send.push_back(town_cargo_copy);
            get_local_pricer()->report_sale(type, price, amount);
            desired -= amount;
            storage[type] -= amount;
            town_cargo->transfer_cargo(amount);
            
            if (town_cargo->amount == 0) {
                it = get_local_pricer()->delete_town_cargo(it);
                if (it == ms.end()) {
                    break;
                }
                town_cargo = (*it).lock();
                seller_price = town_cargo->price;
                if ((buyer_price / (1 + fee)) > seller_price) break;
            } else {
                break; // Didn't buy the full amount, must not afford it all?
            }
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

Dictionary Town::get_local_prices() const {
    std::scoped_lock lock(m);
    return get_local_pricer()->get_local_prices_dict(); 
}

std::unordered_map<int, float> Town::get_local_prices_map() {
    std::scoped_lock lock(m);
    return get_local_pricer()->get_local_prices(); 
}

void Town::buy_cargo(int type, int amount, float price, int p_terminal_id) {
    if (amount <= 0) {
        return;
    }
    TownCargo* new_town_cargo = new TownCargo(type, amount, price, p_terminal_id);
    {
        std::scoped_lock lock(m);
        storage[type] += amount;
        get_local_pricer()->add_town_cargo(new_town_cargo);
    }
}

void Town::buy_cargo(const TownCargo* cargo) {
    TownCargo* new_town_cargo = new TownCargo(cargo);
    if (new_town_cargo->amount <= 0) {
        return;
    }
    int p_terminal_id = cargo->terminal_id;
    int type = cargo->type;
    {
        std::scoped_lock lock(m);
        storage[type] += cargo->amount;
        get_local_pricer()->add_town_cargo(new_town_cargo);

    }
    
}

float Town::add_cargo(int type, float amount) {
    ERR_FAIL_V_EDMSG(0, "Add cargo shouldn't be used on towns");
}

void Town::age_all_cargo() {
    std::unordered_map<int, std::unordered_map<int, int>> cargo_to_return; // Stores money to pay other brokers locally to be done after unlocking
    {
        std::scoped_lock lock(m);
        cargo_to_return = get_local_pricer()->age_all_cargo_and_get_cargo_to_return();
    }   
    
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    for (const auto& [terminal_id, map]: cargo_to_return) {
        Ref<FactoryTemplate> factory = terminal_map->get_terminal_as<FactoryTemplate>(terminal_id);
        if (factory.is_null()) continue;
        for (const auto& [type, amount]: map) {
            factory->add_cargo(type, amount);
            storage[type] -= amount;
        }
    }
}

//Economy Stats

void Town::update_buy_orders() {

    std::unordered_map<int, float> map;
    {
        std::scoped_lock lock(m);
        map = get_local_pricer() -> get_last_month_demand();
    }
    
    for (const auto& [type, amount]: map) {
        if (amount == 0) {
            remove_order(type);
            remove_accept(type);
        } else {
            float price = get_local_price(type);
            edit_order(type, amount, true, price);
            add_accept(type);
        }
    }
}

float Town::get_diff_between_demand_and_supply(int type) const {
    std::scoped_lock lock(m);
    return get_local_pricer()->get_diff_between_demand_and_supply(type);
}
float Town::get_diff_between_demand_and_supply_unsafe(int type) const {
    return get_local_pricer()->get_diff_between_demand_and_supply(type);
}

int Town::get_local_demand(int type) const {
    std::scoped_lock lock(m);
    return get_local_pricer()->get_local_demand(type);
}

// Process Hooks
void Town::day_tick() {
    distribute_cargo();
}

void Town::month_tick() {
    update_buy_orders();
    {
        std::scoped_lock lock(m);
        get_local_pricer() -> update_local_prices();
    }
    age_all_cargo();
}
