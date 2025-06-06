#include "town.hpp"
#include "../singletons/cargo_info.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Town::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location"), &Town::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location"), &Town::initialize);

    // Trade-related
    ClassDB::bind_method(D_METHOD("does_accept", "type"), &Town::does_accept);
    ClassDB::bind_method(D_METHOD("get_local_price", "type"), &Town::get_local_price);
    ClassDB::bind_method(D_METHOD("is_price_acceptable", "type", "price"), &Town::is_price_acceptable);
    ClassDB::bind_method(D_METHOD("get_desired_cargo", "type", "price"), &Town::get_desired_cargo);
    ClassDB::bind_method(D_METHOD("buy_cargo", "type", "amount", "price"), &Town::buy_cargo);
    ClassDB::bind_method(D_METHOD("sell_cargo", "type", "amount", "price"), &Town::sell_cargo);

    // Factory and Pop management
    ClassDB::bind_method(D_METHOD("add_factory", "factory"), &Town::add_factory);
    ClassDB::bind_method(D_METHOD("add_pop", "pop"), &Town::add_pop);
    ClassDB::bind_method(D_METHOD("get_supply"), &Town::get_supply);
    ClassDB::bind_method(D_METHOD("get_demand"), &Town::get_demand);

    // Fulfillment
    ClassDB::bind_method(D_METHOD("get_fulfillment", "type"), &Town::get_fulfillment);
    ClassDB::bind_method(D_METHOD("get_fulfillment_dict"), &Town::get_fulfillment_dict);

    // Selling
    ClassDB::bind_method(D_METHOD("sell_to_pops"), &Town::sell_to_pops);
    ClassDB::bind_method(D_METHOD("sell_type", "type"), &Town::sell_type);
    ClassDB::bind_method(D_METHOD("get_total_pops"), &Town::get_total_pops);
    ClassDB::bind_method(D_METHOD("sell_to_other_brokers"), &Town::sell_to_other_brokers);
    ClassDB::bind_method(D_METHOD("distribute_from_order", "order"), &Town::distribute_from_order);

    // Game Loop
    ClassDB::bind_method(D_METHOD("day_tick"), &Town::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &Town::month_tick);


}

Town::Town(): Broker(Vector2i(0, 0), 0) {
    market = memnew(TownMarket);
}

Town::~Town() {
    if (market != nullptr) {
        memdelete(market);
    }
    for (const auto &[__, pop]: city_pops) {
        memdelete(pop);
    }
    city_pops.clear();
    for (const auto &[__, list]: internal_factories) {
        for (FactoryTemplate* fact: list) {
            memdelete(fact);
        }
    }
    internal_factories.clear();
}

Town::Town(Vector2i new_location): Broker(new_location, 0) {
    market = memnew(TownMarket);
}

Terminal* Town::create(Vector2i new_location) {
    return memnew(Town(new_location));
}

void Town::initialize(Vector2i new_location) {
    market = memnew(TownMarket);
    Broker::initialize(new_location, 0);
}

// Trade
bool Town::does_accept(int type) const {
    return get_cargo_amount(type) != get_max_storage();
}

float Town::get_local_price(int type) const {
    return market -> get_local_price(type);
}

bool Town::is_price_acceptable(int type, float price) const {
    return market -> is_price_acceptable(type, price);
}

int Town::get_desired_cargo(int type, float price) const {
    return market -> get_desired_cargo(type, price);
}

void Town::buy_cargo(int type, int amount, float price) {
    return market -> buy_cargo(type, amount, price);
}

int Town::sell_cargo(int type, int amount, float price) {
    return market -> sell_cargo(type, amount, price);
}

void Town::add_cash(float amount) {
    market -> add_cash(amount);
}
void Town::remove_cash(float amount) {
    market -> remove_cash(amount);
}
float Town::get_cash() const {
    return market -> get_cash();
}

// Production
float Town::get_fulfillment(int type) const {
    return market -> get_fulfillment(type);
}

Dictionary Town::get_fulfillment_dict() const {
    Dictionary d;
    for (int type = 0; type < market->get_supply().size(); type++) {
        d[type] = get_fulfillment(type);
    }
    return d;
}

void Town::add_factory(FactoryTemplate* fact) {
    if (!internal_factories.count(fact->get_player_owner()))
		internal_factories[fact->get_player_owner()] = std::vector<FactoryTemplate*>();
	internal_factories[fact->get_player_owner()].push_back(fact);
	fact->add_connected_broker(this);
}

Dictionary Town::get_supply() const {
    Dictionary d = {};
    const auto v = market -> get_supply();
    for (int type = 0; type < v.size(); type++) {
        d[type] = v[type];
    }
    return d;
}

Dictionary Town::get_demand() const {
    Dictionary d = {};
    const auto v = market -> get_demand();
    for (int type = 0; type < v.size(); type++) {
        d[type] = v[type];
    }
    return d;
}

//Storage
Dictionary Town::get_current_hold() const {
    return market -> get_current_hold();
}
int Town::add_cargo(int type, int amount) {
    return market -> add_cargo(type, amount);
}
void Town::remove_cargo(int type, int amount) {
    market -> remove_cargo(type, amount);
}
int Town::transfer_cargo(int type, int amount) {
    return market -> transfer_cargo(type, amount);
}

//Pop stuff
void Town::add_pop(BasePop* pop) {
    ERR_FAIL_COND_MSG(city_pops.count(pop -> get_pop_id()) != 0, "Pop of id has already been created");
    city_pops[pop -> get_pop_id()] = pop;
}

void Town::sell_to_pops() {
    //market -> get_supply().size() represents all goods
    for (int type = 0; type < market->get_supply().size(); type++) {
        sell_type(type);
    }
}

void Town::update_buy_orders() {
    const auto v = market -> get_demand();
    for (int type = 0; type < v.size(); type++) {
        if (v[type == 0]) {
            remove_order(type);
        } else {
            edit_order(type, v[type], true, market -> get_local_price(type));
        }
    }
}

void Town::sell_type(int type) { // Types are weird
    float amount_sold = 0.0;
    float amount_wanted = 0.0;
	for (const auto &[__, pop]: city_pops) {
		float price = market -> get_local_price(type);
		float amount = pop -> get_desired(type, price); //Float for each pop
        amount_wanted += amount;
        float available_in_market = float(market->get_cargo_amount(type)) - amount_sold;
		amount = std::min(amount, available_in_market);
		amount_sold += amount;
		pop->buy_good(amount, price);
		market->add_cash(amount * price);
    }
    market->report_attempt_to_sell(type, round(amount_wanted)); //Each amount wanted by pops
	market->remove_cargo(type, round(amount_sold));
}

int Town::get_total_pops() const {
    return city_pops.size();
}

FactoryTemplate* Town::find_employment(BasePop* pop) const {
    float max_wage = 0.0;
    FactoryTemplate* best_fact = nullptr;
    
    for (const auto &[__, fact_vector]: internal_factories) {
        for (const auto &fact: fact_vector) {
            if (pop -> will_work_here(fact) && fact -> get_wage() > max_wage)
                best_fact = fact;
                max_wage = fact -> get_wage();
        }
    }
	return best_fact;
}

//Selling to brokers
void Town::sell_to_other_brokers() {
    std::vector<int> supply = market->get_supply();
	for (int type = 0; type < supply.size(); type++) {
		TradeOrder* order = memnew(TradeOrder(type, market->get_cargo_amount(type), false, market->get_local_price(type)));
		distribute_from_order(order);
        memdelete(order);
    }
}

void Town::distribute_from_order(const TradeOrder* order) {
    //Distribute to local factories
	for (const auto &[__, fact_vector]: internal_factories) {
		for (FactoryTemplate* fact: fact_vector) {
			if (fact->does_accept(order->get_type())) {
                distribute_to_order(fact, order);
            }
        }
    }
	//Distribute to stations, ports, or other brokers
	for (const auto &[__, broker]: connected_brokers) {
		if (broker->does_accept(order->get_type())) {
            distribute_to_order(broker, order);
        }
    }
}

void Town::report_attempt_to_sell(int type, int amount) {
    //Only called from selling, so amount is negitive, therefore swap it back to pos and report
    market -> report_attempt_to_sell(type, amount);
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

// Process Hooks
void Town::day_tick() {
    sell_to_other_brokers();
}

void Town::month_tick() {
    sell_to_pops();
    update_buy_orders();
    market -> month_tick();
}
