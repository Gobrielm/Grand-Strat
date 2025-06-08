#pragma once

#include <unordered_map>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class CargoInfo : public RefCounted {
    GDCLASS(CargoInfo, RefCounted);

private:
    static Ref<CargoInfo> singleton_instance;

    std::vector<String> cargo_names = {
        "clay", "sand", "sulfur", "lead", "iron", "coal", "copper", "zinc", "wood", "salt", 
        "grain", "livestock", "fish", "fruit", "cotton", "silk", "spices", "coffee", "tea", "tobacco", 
        "gold",
        
        "bricks", "glass", "lumber", "paper", "tools", "steel", "brass", "dynamite",
        "flour", "fabric", "liquor", "bread", "leather", "meat", "clothes",
        "wine", "luxury_clothes", "preserved_fruit", "porcelain",
        "furniture", "wagons", "boats", "lanterns", "trains",
        "ammo", "guns", "artillery", "preserved_meat", "canned_food", "rations", "luxury_rations",
    };
    std::unordered_map<std::string, int> cargo_types = {};
    const std::unordered_map<std::string, float> base_prices = {
    {"clay", 10.0f}, {"sand", 10.0f}, {"sulfur", 10.0f}, {"lead", 10.0f}, {"iron", 10.0f},
    {"coal", 10.0f}, {"copper", 10.0f}, {"zinc", 10.0f}, {"wood", 10.0f}, {"salt", 10.0f},
    {"grain", 10.0f}, {"livestock", 10.0f}, {"fish", 10.0f}, {"fruit", 10.0f}, {"cotton", 10.0f},
    {"silk", 10.0f}, {"spices", 10.0f}, {"coffee", 10.0f}, {"tea", 10.0f}, {"tobacco", 10.0f},
    {"gold", 10.0f}, {"bricks", 10.0f}, {"glass", 10.0f}, {"lumber", 10.0f}, {"paper", 10.0f},
    {"tools", 10.0f}, {"steel", 10.0f}, {"brass", 10.0f}, {"dynamite", 10.0f}, {"flour", 10.0f},
    {"fabric", 10.0f}, {"liquor", 10.0f}, {"bread", 10.0f}, {"leather", 10.0f}, {"meat", 10.0f},
    {"clothes", 10.0f}, {"wine", 10.0f}, {"luxury_clothes", 10.0f}, {"preserved_fruit", 10.0f},
    {"porcelain", 10.0f}, {"furniture", 10.0f}, {"wagons", 10.0f}, {"boats", 10.0f},
    {"lanterns", 10.0f}, {"trains", 10.0f}, {"ammo", 10.0f}, {"guns", 10.0f}, {"artillery", 10.0f},
    {"preserved_meat", 10.0f}, {"canned_food", 10.0f}, {"rations", 10.0f}, {"luxury_rations", 10.0f}
    };


protected:
    static void _bind_methods();

public:
    int amount_of_primary_goods = 0;
    
    CargoInfo();
    static void initialize_singleton();
    
    static Ref<CargoInfo> get_instance();

    const std::unordered_map<int, float> get_base_prices();
    String get_cargo_name(int type) const;
    int get_cargo_type(String cargo_name) const;
    void create_amount_of_primary_goods();
    int get_number_of_goods() const;
    bool is_cargo_primary(int cargo_type) const;
    Array get_cargo_array() const;
};
