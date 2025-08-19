#include "town_local_price_controller.hpp"
#include "../../singletons/cargo_info.hpp"

using TLPC = TownLocalPriceController;
using ms_it = std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare>::iterator;

TLPC::TownLocalPriceController(): LocalPriceController() {

}

void TLPC::add_town_cargo(TownCargo* new_cargo) {
    int term_id = new_cargo->terminal_id;
    int type = new_cargo->type;
    if (TownCargo* existing_cargo = get_cargo(term_id, type)) {
        existing_cargo->price = new_cargo->price;
        existing_cargo->age = 0;
        existing_cargo->amount += new_cargo->amount;
        delete new_cargo; // Don't need it anymore
    } else {
        cargo_sell_orders[type].insert(new_cargo);
        town_cargo_tracker[new_cargo->terminal_id][type] = new_cargo;
    }
}

TownCargo* TLPC::get_cargo(int term_id, int type) const {
    if (town_cargo_tracker.count(term_id)) {
        const auto& cargo_map = (town_cargo_tracker.at(term_id));
        const auto it = cargo_map.find(type);
        return it == cargo_map.end() ? nullptr : it->second;
    } 
    return nullptr;
}

std::unordered_map<int, std::unordered_map<int, int>> TLPC::age_all_cargo_and_get_cargo_to_return() {
    std::unordered_map<int, std::unordered_map<int, int>> cargo_to_return; // Stores money to pay other brokers locally to be done after unlocking
    {
        for (auto& [__, ms]: cargo_sell_orders) {
            for (auto it = ms.begin(); it != ms.end();) { // No iterator
                it = return_cargo(it, cargo_to_return);
            }
        }
    }   
    return cargo_to_return;
}

ms_it TLPC::return_cargo(ms_it cargo_it, std::unordered_map<int, std::unordered_map<int, int>>& cargo_to_return) {
    TownCargo* cargo = (*cargo_it);
    if ((++(cargo->age)) > 5) {
        int type = cargo->type;
        cargo->return_cargo(cargo_to_return);       //Return cargo to broker
        cargo_it = delete_town_cargo(cargo_it);     //Delete old cargo
    } else {
        cargo_it++;                  //Increment if not deleting    
    }
    return cargo_it;
}

ms_it TLPC::delete_town_cargo(ms_it &sell_order_it) {
    TownCargo* sell_order = *sell_order_it;
    town_cargo_tracker[sell_order->terminal_id].erase(sell_order->type); // Erases from tracker map
    sell_order_it = cargo_sell_orders[sell_order->type].erase(sell_order_it); // Erases from price ordered map
    delete sell_order; // Deletes from memory
    return sell_order_it; // Returns shifted pointer
}

void TLPC::update_local_price(int type) { // Creates a weighted average of cargo sold

    if (cargo_sell_orders[type].size() != 0) { // If there is demand + supply but no sales
        current_prices[type] = get_weighted_average(cargo_sell_orders[type]); // Take average of sell orders, sellers will lower if no sales, and buyers will come up too
    } else if (cargo_sold_map[type].size() != 0) {
        current_prices[type] = get_weighted_average(cargo_sold_map[type]);
        cargo_sold_map[type].clear();
    }
    ERR_FAIL_COND_MSG(std::isnan(current_prices[type]), "Type: " + CargoInfo::get_instance()->get_cargo_name(type) + " has null price!");
    
}

double TLPC::get_weighted_average(std::multiset<TownCargo*, TownCargo::TownCargoPtrCompare> &s) const { // Huge amount of cargo with 0 cargo
    int total_weight = 0;
    float sum_of_weighted_terms = 0;
    for (const auto& cargo: s) {
        int amount = cargo->amount;
        float price = cargo->price;
        float weighted_ave = price * amount;
        total_weight += amount;
        sum_of_weighted_terms += weighted_ave;
    }
    ERR_FAIL_COND_V_MSG(total_weight == 0, 0.0, "Total Amount sold is 0 yet was not erased!");

    return sum_of_weighted_terms / total_weight;
}

double TLPC::get_weighted_average(std::unordered_map<int, int> &m) const {
    float total_weight = 0;
    double sum_of_weighted_terms = 0;
    for (const auto& [ten_price, amount]: m) {
        float weighted_ave = (ten_price / 10.0) * amount;
        total_weight += amount;
        sum_of_weighted_terms += weighted_ave;
    }   
    ERR_FAIL_COND_V_MSG(total_weight == 0, 0.0, "Total Amount sold is 0 yet was not erased!");
    return sum_of_weighted_terms / total_weight;
}