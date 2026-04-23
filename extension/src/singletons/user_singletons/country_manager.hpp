#pragma once

#include <unordered_map>
#include <mutex>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/ref.hpp>

class Country;

using namespace godot;

class CountryManager : public RefCounted {
    GDCLASS(CountryManager, RefCounted);

    mutable std::mutex m;
    static Ref<CountryManager> singleton_instance;
    std::unordered_map<int, Ref<Country>> countries;
    

protected:
    static void _bind_methods();

public:
    CountryManager();
    
    static void create();
    static void cleanup();
    static Ref<CountryManager> get_instance();

    void create_country(int country_id);
    void assign_country_to_player_id(int country_id, int player_id);
    void delete_country(int id);

    Ref<Country> get_country(int p_id) const;

    void month_tick();
};
