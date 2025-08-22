#pragma once
#include "../local_price_controller.hpp"
#include "town_cargo.hpp"
#include <memory>

class TownLocalPriceController: public LocalPriceController {

    protected:
    std::unordered_map <int, int> local_demand;
    std::unordered_map <int, int> last_month_local_demand;

    std::unordered_map<int, std::unordered_map<int, int>> cargo_sold_map; // type -> price * 10 -> amount
    std::unordered_map<int, std::unordered_map<int, std::shared_ptr<TownCargo>>> town_cargo_tracker; // Owner id -> type -> TownCargo*

    std::shared_ptr<TownCargo> get_cargo(int term_id, int type) const;

    using ms_it = std::multiset<std::weak_ptr<TownCargo>, TownCargo::TownCargoPtrCompare>::iterator;
    ms_it age_cargo(ms_it cargo_it, std::unordered_map<int, std::unordered_map<int, int>>& cargo_to_return);
    
    double get_weighted_average(std::multiset<std::weak_ptr<TownCargo>, TownCargo::TownCargoPtrCompare> &s) const;
    double get_weighted_average(std::unordered_map<int, int> &m) const;
    void update_local_price(int type) override;
    public:
    std::unordered_map<int, std::multiset<std::weak_ptr<TownCargo>, TownCargo::TownCargoPtrCompare>> cargo_sell_orders; // Lowest price first

    TownLocalPriceController();
    void add_local_demand(int type, int amount);
    void report_sale(int type, float price, float amount);
    float get_local_demand(int type) const;
    float get_diff_between_demand_and_supply(int type) const;
    float get_demand_at_price(int type, float price) const override;
    std::unordered_map<int, float> get_demand_at_different_prices(int type) const; // Returns with ten_price -> amount

    void add_town_cargo(TownCargo* new_cargo);
    // Takes storage and takes from it, and returns a map of cargo to return
    std::unordered_map<int, std::unordered_map<int, int>> age_all_cargo_and_get_cargo_to_return();
    ms_it delete_town_cargo(ms_it sell_order_it);
    void update_local_prices() override;

    

};