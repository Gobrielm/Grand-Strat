#pragma once

#include <unordered_map>
#include <mutex>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class Country : public RefCounted {
    GDCLASS(Country, RefCounted);

private:
    const int id;
    int player_id;
    float money;

protected:
    static void _bind_methods();

public:
    Country(int p_id = -1);
    
    static Ref<Country> create_instance(int p_id = -1);

    void add_money(float amount);
    void remove_money(float amount);
    float get_money() const;
    float transfer_money(float amount);
    bool has_enough_money(int amount) const;

    void assign_player_id(int p_player_id);
    
};
