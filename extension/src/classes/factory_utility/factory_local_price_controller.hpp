#pragma once

#include "local_price_controller.hpp"
#include <unordered_map>

using namespace godot;

class FactoryTemplate;

class FactoryLocalPriceController: public LocalPriceController {

    protected:
    FactoryTemplate* factory;
    void update_local_price(int type);
    template <typename Compare>
    double get_weighted_average(std::map<int, float, Compare> &m, int amount_to_stop_at) const; // Stops early to potentially expect a better price
    

    public:

    FactoryLocalPriceController(FactoryTemplate* p_factory);
};
