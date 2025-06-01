#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <unordered_map>

using namespace godot;

class LocalPriceController: public RefCounted {
    GDCLASS(LocalPriceController, RefCounted);
    
    static const float MAX_DIFF;
    static std::unordered_map<int, float> base_prices;

    std::unordered_map<int, int> change;
    std::unordered_map<int, int> attempts_to_trade;
    std::unordered_map<int, float> local_prices;

    float get_multiple(int type) const;

    void vary_buy_order(int demand, int supply, int type);
    void vary_sell_order(int demand, int supply, int type);
    void bump_up_good_price(int type, float percentage_met, int amount);
    void bump_down_good_price(int type, float percentage_met, int amount);
    void equalize_good_price(int type);

    protected:
    static void _bind_methods();

    public:
    LocalPriceController();
    LocalPriceController(const std::unordered_map<int, int>& inputs, const std::unordered_map<int, int>& outputs);

    static std::unordered_map<int, float>& get_base_prices();
    static void set_base_prices(const Dictionary& p_base_prices);

    void add_cargo_type(int type, float starting_price = -1.0f);
    void remove_cargo_type(int type);
    void add_cargo_from_factory(const std::unordered_map<int, int>& outputs);

    int get_change(int type) const;
    void reset_change(int type);
    void report_change(int type, int amount);
    void report_attempt(int type, int amount);
    void reset_attempts(int type);
    int get_attempts(int type) const;

    float get_local_price(int type) const;
    float get_base_price(int type) const;
    float get_percent_difference(int type) const;

    void vary_input_price(int demand, int type);
    void vary_output_price(int supply, int type);

    Dictionary get_local_prices();
};
