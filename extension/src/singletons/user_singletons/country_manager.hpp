#pragma once

#include "country_instance.hpp"

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
    static constexpr int INITIAL_AMOUNT_OF_MONEY = 10000;

protected:
    static void _bind_methods();

public:
    CountryManager(const Array& peers = Array());
    
    static void create(const Array& peers);
    static Ref<CountryManager> get_instance();

    void add_country(int new_id);
    void delete_country(int id);

    Ref<Country> get_country(int p_id) const;
};
