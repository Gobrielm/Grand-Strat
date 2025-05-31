#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include "terminal.hpp"

using namespace godot;

class Firm : public Terminal {
    GDCLASS(Firm, Terminal);

    protected:
    static void _bind_methods();

    public:

    int get_amount_can_buy(const float amount_per) const;
    void add_cash(float amount);
    void remove_cash(float amount);
    float get_cash() const;
    float transfer_cash(float amount);
    
    static Terminal* create(const Vector2i p_location, const int p_owner);
    void initialize(const Vector2i p_location = Vector2i(), const int p_owner = 0);

    Firm(): Terminal() {}
    Firm(const Vector2i p_location, const int p_owner): Terminal(p_location, p_owner) {}	
};