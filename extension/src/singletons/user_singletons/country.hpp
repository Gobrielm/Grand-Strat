#pragma once

#include <unordered_map>
#include <mutex>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class Country : public RefCounted {
    GDCLASS(Country, RefCounted);

private:
    static constexpr int DEFAULT_MINTING = 5000; 
    const int country_id;
    int player_id;
    int minting;
    
protected:
    static void _bind_methods();

public:
    Country(int p_country_id = -1);
    
    static Ref<Country> create_instance();

    int get_country_id() const;

    void assign_player_id(int p_player_id);
    void month_tick();
    
};
