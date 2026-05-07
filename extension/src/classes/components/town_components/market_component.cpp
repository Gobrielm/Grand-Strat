#include <deque>

#include "market_component.hpp"
#include "singletons/terminal_map.hpp"
#include "classes/province.hpp"
#include "classes/base_pop.hpp"
#include "classes/components/capital_component.hpp"
#include "classes/components/storage_component.hpp"

MarketComponent::MarketComponent() {}

MarketComponent::MarketComponent(const MarketComponent& other): 
    sell_orders(other.sell_orders), 
    buy_orders(other.buy_orders)
{}

void MarketComponent::add_order(std::shared_ptr<TradeOrder> to) {
    if (to->is_buy_order()) {
        buy_orders[to->get_type()].insert(to);
    } else {
        sell_orders[to->get_type()].insert(to);
    }
}

float MarketComponent::get_price(int type) const {
    if (sell_orders.count(type) && !sell_orders.at(type).empty()) {
        if (buy_orders.count(type) && !buy_orders.at(type).empty()) {
            return ((*sell_orders.at(type).begin())->get_price() + (*buy_orders.at(type).begin())->get_price()) / 2.0f;
        } else {
            return (*sell_orders.at(type).begin())->get_price();
        }
    }
    if (buy_orders.count(type) && !buy_orders.at(type).empty()) {
        return (*buy_orders.at(type).begin())->get_price();
    }
    return LocalPriceController::get_base_price(type);
}

float MarketComponent::get_min_price(int type) const {
    if (buy_orders.count(type) && !buy_orders.at(type).empty()) {
        return (*buy_orders.at(type).end())->get_price();
    }
    return 0;
}

float MarketComponent::get_max_price(int type) const {
    if (sell_orders.count(type) && !sell_orders.at(type).empty()) {
        return (*sell_orders.at(type).end())->get_price();
    }
    return 10000;
}

std::unordered_map<int, float> MarketComponent::get_current_prices() const {
    std::unordered_map<int, float> to_return;

    for (const auto& [type, _]: sell_orders) {
        to_return[type] = get_price(type);
    }

    for (const auto& [type, _]: buy_orders) {
        to_return[type] = get_price(type);
    }

    return to_return;
}

std::pair<std::vector<std::pair<int, float>>, std::vector<std::pair<int, float>>> MarketComponent::get_market_info_plot(int type) const {
    std::vector<std::pair<int, float>> buys;
    std::vector<std::pair<int, float>> sells;

    if (buy_orders.count(type) != 0 || sell_orders.count(type) != 0) {
        return std::make_pair(buys, sells);
    }

    for (auto it = buy_orders.at(type).begin(); it != buy_orders.at(type).end(); it++) {
        auto& ptr = (*it);
        if (ptr->get_type() != type || ptr->get_amount() == 0) continue;
        buys.push_back(std::make_pair(ptr->get_amount(), ptr->get_price()));
    }

    for (auto it = sell_orders.at(type).begin(); it != sell_orders.at(type).end(); it++) {
        auto& ptr = (*it);
        if (ptr->get_type() != type || ptr->get_amount() == 0) continue;
        buys.push_back(std::make_pair(ptr->get_amount(), ptr->get_price()));
    }
    return std::make_pair(buys, sells);
}

Array MarketComponent::get_market_info_plot_godot(int type) const {
    Array buys;
    Array sells;

    if (last_month_plot.count(type) != 0) {
        return Array();
    }

    const auto& orders = last_month_plot.at(type);

    const auto& last_month_buy_orders = orders.first;
    const auto& last_month_sell_orders = orders.second;
    
    for (const auto& amt_price: last_month_buy_orders) {
        Vector2 v(amt_price.first, amt_price.second);
        buys.push_back(v);
    }

    for (const auto& amt_price: last_month_sell_orders) {
        Vector2 v(amt_price.first, amt_price.second);
        sells.push_back(v);
    }
    

    Array a;
    a.push_back(buys);
    a.push_back(sells);
    return a;
}

void MarketComponent::market_tick(Province* province) {

    for (auto& [type, buys]: buy_orders) {

        auto it = sell_orders[type].begin();
        if (it == sell_orders[type].end()) continue;

        for (auto& buy_order: buys) {
            while (it != sell_orders[type].end() && (*it)->get_amount() == 0) {
                it++;
            }
            auto& sell_order = *it;

            float price1 = buy_order->get_price();
            float price2 = (*it)->get_price();
            float avg = (price1 + price2) / 2.0f;

            if (!buy_order->is_price_acceptable(avg) || !sell_order->is_price_acceptable(avg)) {
                continue;
            }
            
            finish_market_exchange(
                buy_order, sell_order,
                province->get_capital_and_storage_components_unsafe(buy_order),
                province->get_capital_and_storage_components_unsafe(sell_order)
            );
        }
    }
}

void MarketComponent::finish_market_exchange(
    std::shared_ptr<TradeOrder> buy_order, 
    std::shared_ptr<TradeOrder> sell_order, 
    std::pair<CapitalComponent*, StorageComponent*> buyer, 
    std::pair<CapitalComponent*, StorageComponent*> seller
) {
    int type = buy_order->get_type();
    float price1 = buy_order->get_price();
    float price2 = sell_order->get_price();
    float price = (price1 + price2) / 2.0f;
    int amt = std::min(std::min(buy_order->get_amount(), sell_order->get_amount()), (unsigned) int(seller.second->get_amount(type)));

    // Seller doesn't have enough
    if (amt <= 0) {
        return;
    }

    float sub_total = price * amt;

    // Buyer can't afford
    if (buyer.first->get_cash() < sub_total) {
        return;
    }

    buyer.first->remove_cash(sub_total);
    seller.first->add_cash(sub_total);

    buyer.second->add_cargo(type, amt);
    seller.second->remove_cargo(type, amt);
}

int32_t MarketComponent::get_current_demand(int type) const {
    int32_t tot = 0;
    if (!last_month_plot.count(type)) return 0;
    for (const auto& [amt, price]: last_month_plot.at(type).first) {
        tot += amt;
    }
    return tot;
}

int32_t MarketComponent::get_current_supply(int type) const {
    int32_t tot = 0;
    if (!last_month_plot.count(type)) return 0;
    for (const auto& [amt, price]: last_month_plot.at(type).second) {
        tot += amt;
    }
    return tot;
}

void MarketComponent::sell_to_pop(Province* province, BasePop& pop) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    // std::unordered_map<int, float> money_to_pay;
    
    for (auto& [type, orders]: sell_orders) {
        unsigned int desired = pop.get_desired(type);
        if (desired == 0) {
            continue;
        }
        float pop_price = pop.get_buy_price(type, get_price(type));

        // get_local_pricer() -> add_demand(type, pop_price, desired); // Only add demand since its not part of survey_broad_market()
        // get_local_pricer() -> add_local_demand(type, desired);

        for (auto sell_it = orders.begin(); sell_it != orders.end(); sell_it++) {
            auto& order = *sell_it;
            if (order->get_amount() == 0) continue;

            // demand_to_relay.push_back(std::unique_ptr<TownCargo>(new TownCargo(type, desired, pop_price, sell_order->terminal_id)));

            const float seller_price = order->get_price();
            const float buyer_price = pop.get_buy_price(type, seller_price);

            if (std::isnan(seller_price)) {
                ERR_FAIL_MSG("seller price is nan");
            }
            if (std::isnan(buyer_price)) {
                ERR_FAIL_MSG("buyer_price is nan");
            }

            float price = (buyer_price + seller_price) / 2.0f; // Price average

            if (((price / buyer_price)) > 1.15) { // Too high for buyer no deal
                break;
            }
            
            if (!order->is_price_acceptable(price)) { // Too low for seller no deal
                break;
            }

            unsigned int amt = std::min(pop.get_desired(type, price), order->get_amount());
            if (amt == 0) break; 

            // get_local_pricer()->report_sale(type, price, amount);
            // sell_order->sell_cargo(amount, price, money_to_pay); // Calls with money_to_pay
            // pop.buy_good(type, amount, price);

            // if (sell_order->amount == 0) {
            //     sell_it = get_local_pricer()->delete_town_cargo(sell_it);
            // } else {
            //     break;
            // }get_source_id

            auto res = province->get_capital_and_storage_components_unsafe(order);
            if (res.first == nullptr || res.second == nullptr) {
                ERR_FAIL_MSG("order's owner is invalid");
            }
            auto [capital, storage] = res;
            amt = std::min(amt, (unsigned) floor(storage->get_amount(type)));
            if (amt == 0) continue;

            float sub_total = price * amt;

            capital->add_cash(sub_total);
            storage->remove_cargo(type, amt);
            pop.buy_good(type, amt, price);
        }
    }
    

    // for (const auto &[terminal_id, to_pay]: money_to_pay) {
    //     Ref<Broker> broker = terminal_map->get_terminal_as<Broker>(terminal_id);
    //     if (broker.is_null()) continue;
    
    //     broker->add_cash(to_pay);
    // }

    // for (const auto &demand_cargo: demand_to_relay) {
    //     Ref<Broker> broker = terminal_map->get_terminal_as<Broker>(demand_cargo->terminal_id);
    //     if (broker.is_null()) continue;
    
    //     broker->add_surveyed_demand(demand_cargo->type, demand_cargo->price, demand_cargo->amount);
    // }
}

void MarketComponent::update_last_month_plot() {
    last_month_plot.clear();
    
    for (auto& [type, _]: buy_orders) {
        last_month_plot[type] = get_market_info_plot(type);
    }
}

float MarketComponent::find_market_price_equilibrium(int type) const {
    if (!last_month_plot.count(type)) return 0;


    int supply_included = 0;
    float current_price = 0;
    auto lowest_price_included = last_month_plot.at(type).first.cend() - 1;
    auto lowest_price_limit = last_month_plot.at(type).first.cbegin() - 1;

    for (const auto& [amt, price]: last_month_plot.at(type).first) {
        supply_included += amt;
        current_price = std::min(current_price, price);
    }

    int demand_included = 0;

    for (const auto& [amt, price]: last_month_plot.at(type).second) {


        while (lowest_price_limit != lowest_price_included, lowest_price_included->second < price) {
            supply_included -= lowest_price_included->first;
            lowest_price_included--;
        }
    }

    return 0;
}