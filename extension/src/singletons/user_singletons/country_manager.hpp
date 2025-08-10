#pragma once

#include "country.hpp"

#include <unordered_map>
#include <mutex>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class CountryManager : public RefCounted {
    GDCLASS(CountryManager, RefCounted);

private:
    mutable std::mutex m;
    std::unordered_map<int, Ref<Country>> countries;
    static Ref<CountryManager> singleton_instance;

protected:
    static void _bind_methods();

public:
    CountryManager();
    
    static void create();
    static Ref<CountryManager> get_instance();

    void create_country(int country_id);
    void assign_country_to_player_id(int country_id, int player_id);
    void delete_country(int id);

    Ref<Country> get_country(int p_id) const;

    void month_tick();
};
