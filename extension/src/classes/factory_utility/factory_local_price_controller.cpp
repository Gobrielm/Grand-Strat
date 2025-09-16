#include "factory_local_price_controller.hpp"
#include "../factory_template.hpp"

FactoryLocalPriceController::FactoryLocalPriceController(FactoryTemplate* p_factory): LocalPriceController() {
    factory = p_factory;
}

void FactoryLocalPriceController::update_local_price(int type) {

}

template <typename Compare>
double FactoryLocalPriceController::get_weighted_average(std::map<int, float, Compare> &m, int amount_to_stop_at) const {
    float total_weight = 0;
    double sum_of_weighted_terms = 0;
    for (const auto& [ten_price, amount]: m) {
        amount = std::min(amount, amount_to_stop_at - total_weight);
        float weighted_ave = (ten_price / 10.0) * amount;
        total_weight += amount;
        sum_of_weighted_terms += weighted_ave;

        if (total_weight >= amount_to_stop_at) break;
        
    }   
    ERR_FAIL_COND_V_MSG(total_weight == 0, 0.0, "Total Amount sold is 0 yet was not erased!");
    return sum_of_weighted_terms / total_weight;
}