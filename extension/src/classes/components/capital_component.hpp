#pragma once

#include <godot_cpp/variant/vector2.hpp>
#include <list>

using namespace godot;

class CapitalComponent {
    private:
    float cash;
    std::list<float> cash_history;
    static constexpr float INITIAL_CASH = 1000;

    public:

    CapitalComponent();

    int get_amount_can_buy(const float amount_per) const;
    void add_cash(float amount);
    void remove_cash(float amount);
    float get_cash() const;
    float transfer_cash(float amount);
    void update_cash_history();
    const std::list<float>& get_cash_history() const;
    const float get_recent_change() const;

};