#pragma once

#include "classes/local_price_controller.hpp"
#include <unordered_map>
#include <memory>

class StorageComponent {

    private:
    static constexpr int INITIAL_MAX_STORAGE = 5000;

    std::unordered_map<int, float> storage;

    public:
    float MAX_STORAGE;
    
    StorageComponent();

    float get_desired_cargo(int type, float pricePer);

    float get_amount(int type);

    /// @return The amount of cargo not added to local storage
    float add_cargo(int type, float amount);
    void remove_cargo(int type, float amount);

    godot::Dictionary dictionary() const;
};
