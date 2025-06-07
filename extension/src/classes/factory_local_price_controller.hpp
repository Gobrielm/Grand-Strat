#pragma once

#include "local_price_controller.hpp"
#include <unordered_map>

using namespace godot;

class FactoryLocalPriceController: public LocalPriceController {
    GDCLASS(FactoryLocalPriceController, LocalPriceController);

    protected:
    static void _bind_methods();

    public:
    static constexpr float MAX_DIFF = 1.4f; //Highest difference from base price for all traders

    FactoryLocalPriceController();

    float get_max_diff() const override;
};
