#pragma once
#include "../local_price_controller.hpp"
#include "town_cargo.hpp"

class TownLocalPriceController: public LocalPriceController {

    protected:
    std::unordered_map <int, int> local_demand;
    std::unordered_map <int, int> last_month_local_demand;

    std::unordered_map<int, std::unordered_map<int, int>> cargo_sold_map; // type -> price * 10 -> amount
    std::unordered_map<int, std::multiset<TownCargo*, TownCargo::TownCargoPtrCompare>> cargo_sell_orders; // Lowest price first
    std::unordered_map<int, std::unordered_map<int, TownCargo*>> town_cargo_tracker; // Owner id -> type -> TownCargo*

    TownCargo* get_cargo(int term_id, int type) const;

    using ms_it = std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare>::iterator;
    ms_it return_cargo(ms_it cargo_it, std::unordered_map<int, std::unordered_map<int, int>>& cargo_to_return);
    ms_it delete_town_cargo(ms_it &sell_order_it);
    
    void update_local_price(int type);
    double get_weighted_average(std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare> &s) const;
    double get_weighted_average(std::unordered_map<int, int> &m) const;

    public:
    TownLocalPriceController();

    void add_town_cargo(TownCargo* new_cargo);
    std::unordered_map<int, std::unordered_map<int, int>> age_all_cargo_and_get_cargo_to_return();

};