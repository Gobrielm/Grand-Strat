#pragma once
#include <unordered_map>
#include <godot_cpp/variant/dictionary.hpp>

class ConstructionComponent {
    private:

    std::unordered_map<int, float> materials;
    std::unordered_map<int, float> max_amounts;

    public:
    
    ConstructionComponent(std::unordered_map<int, float> p_max_amounts = std::unordered_map<int, float>()) {
        max_amounts = p_max_amounts;
        for (const auto& [key, _]: max_amounts) {
            materials[key] = 0;
        }
    }

    void add_construction_material(int type, int amount);

    godot::Dictionary get_construction_materials_godot() const;
    godot::Dictionary get_total_construction_materials_godot() const;

    bool is_needed_for_construction(int type) const;
    int get_amount_of_type_needed_for_construction(int type) const;
    bool is_finished_constructing() const;
    bool is_constructing() const;

    int get_desired_cargo(int type, float pricePer) const;


    /// @return The amount of cargo not added to local storage
    float add_cargo(int type, float amount);
    void remove_cargo(int type, float amount);

    void finish_construction();
    
};
