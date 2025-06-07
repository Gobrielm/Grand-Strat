#include "factory_local_price_controller.hpp"


void FactoryLocalPriceController::_bind_methods() {}


FactoryLocalPriceController::FactoryLocalPriceController(): LocalPriceController() {}

float FactoryLocalPriceController::get_max_diff() const {
    return MAX_DIFF;
}