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

std::vector<int> MarketComponent::get_types_among_orders() const {
    std::vector<int> v;
    std::unordered_set<int> types_added;
    for (auto order: buy_orders) {
        v.push_back(order->get_type());
        types_added.insert(order->get_type());
    }
    for (auto order: sell_orders) {
        if (!types_added.count(order->get_type())) {
            v.push_back(order->get_type());
        }
    }
    return v;
}

void MarketComponent::add_order(std::shared_ptr<TradeOrder> to) {
    if (to->is_buy_order()) {
        buy_orders.push_back(to);
    } else {
        sell_orders.push_back(to);
    }
}

float MarketComponent::get_price(int type) const {
    if (equilibrium_prices.count(type)) {
        return equilibrium_prices.at(type);
    }
    return LocalPriceController::get_base_price(type);
}

float MarketComponent::get_min_price(int type) const {
    if (sorted_buy_orders.count(type) && !sorted_buy_orders.at(type).empty()) {
        return (sorted_buy_orders.at(type).end())->get_price();
    }
    return 0.0001;
}

float MarketComponent::get_max_price(int type) const {
    if (sorted_sell_orders.count(type) && !sorted_sell_orders.at(type).empty()) {
        return (sorted_sell_orders.at(type).end())->get_price();
    }
    return 10000;
}

std::unordered_map<int, float> MarketComponent::get_current_prices() const {
    std::unordered_map<int, float> to_return;

    for (const auto& type: get_types_among_orders()) {
        to_return[type] = get_price(type);
    }

    return to_return;
}

std::pair<std::vector<std::pair<int, float>>, std::vector<std::pair<int, float>>> MarketComponent::get_market_info_plot(int type) const {
    std::vector<std::pair<int, float>> buys;
    std::vector<std::pair<int, float>> sells;

    if (sorted_buy_orders.count(type)) {
        const auto& orders = sorted_buy_orders.at(type);
        for (auto& order: orders) {
            if (order.get_amount() == 0) continue;
            buys.push_back(std::make_pair(order.get_amount(), order.get_price()));
        }
    }

    if (sorted_sell_orders.count(type)) {
        const auto& orders = sorted_sell_orders.at(type);
        for (auto& order: orders) {
            if (order.get_amount() == 0) continue;
            buys.push_back(std::make_pair(order.get_amount(), order.get_price()));
        }
    }
    return std::make_pair(buys, sells);
}

Array MarketComponent::get_market_info_plot_godot(int type) const {
    Array buys;
    Array sells;

    if (!last_month_plot.count(type)) {
        return Array();
    }

    const auto& orders = last_month_plot.at(type);

    const auto& last_month_buy_orders = orders.first;
    const auto& last_month_sell_orders = orders.second;
    
    for (const auto& [amt, price]: last_month_buy_orders) {
        Vector2 v(amt, price);
        buys.push_back(v);
    }

    for (const auto& [amt, price]: last_month_sell_orders) {
        Vector2 v(amt, price);
        sells.push_back(v);
    }
    

    Array a;
    a.push_back(buys);
    a.push_back(sells);
    return a;
}

void MarketComponent::market_tick(Province* province) {
    // sort orders
    sort_orders();
    
    for (auto& [type, buys]: sorted_buy_orders) {
        auto& sell_orders_vec = sorted_sell_orders[type];

        int i = 0;
        if (sell_orders_vec.empty()) continue;

        for (auto& buy_order: buys) {
            if (buy_order.get_amount() == 0) continue;

            while (i < sell_orders_vec.size() && (sell_orders_vec[i]).get_amount() == 0) {
                i++;
            }
            if (i >= sell_orders_vec.size()) break;
            auto& sell_order = sell_orders_vec[i];

            float price1 = buy_order.get_price();
            float price2 = sell_order.get_price();
            float avg = (price1 + price2) / 2.0f;

            if (!buy_order.is_price_acceptable(avg) || !sell_order.is_price_acceptable(avg)) {
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

void MarketComponent::sort_orders() {
    sorted_buy_orders.clear();
    sorted_sell_orders.clear();
    
    for (auto& order: buy_orders) {
        int type = order->get_type();
        sorted_buy_orders[type].push_back(*order);
    }

    for (auto& order: sell_orders) {
        int type = order->get_type();
        sorted_sell_orders[type].push_back(*order);
    }

    for (auto& [type, orders]: sorted_buy_orders) {
        std::sort(orders.begin(), orders.end(), TradeOrder::TradeOrderGT());
    }
    for (auto& [type, orders]: sorted_sell_orders) {
        std::sort(orders.begin(), orders.end(), TradeOrder::TradeOrderLT());
    }
}

void MarketComponent::finish_market_exchange(
    const TradeOrder& buy_order, 
    const TradeOrder& sell_order, 
    std::pair<CapitalComponent*, StorageComponent*> buyer, 
    std::pair<CapitalComponent*, StorageComponent*> seller
) {
    if (buyer.first == nullptr || buyer.second == nullptr || seller.first == nullptr || seller.second == nullptr) {
        print_error("Null Capital or Storage comp");
        return;
    }
    print_line("T");
    int type = buy_order.get_type();
    float price1 = buy_order.get_price();
    float price2 = sell_order.get_price();
    float price = (price1 + price2) / 2.0f;
    int amt = std::min(std::min(buy_order.get_amount(), sell_order.get_amount()), (unsigned) int(seller.second->get_amount(type)));

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

// void MarketComponent::sell_to_pop(Province* province, BasePop& pop) {
//     Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
//     // std::unordered_map<int, float> money_to_pay;
    
//     for (auto& [type, orders]: sell_orders) {
//         unsigned int desired = pop.get_desired(type);
//         if (desired == 0) {
//             continue;
//         }
//         float pop_price = pop.get_buy_price(type, get_price(type));

//         // get_local_pricer() -> add_demand(type, pop_price, desired); // Only add demand since its not part of survey_broad_market()
//         // get_local_pricer() -> add_local_demand(type, desired);

//         for (auto sell_it = orders.begin(); sell_it != orders.end(); sell_it++) {
//             auto& order = *sell_it;
//             if (order->get_amount() == 0) continue;

//             // demand_to_relay.push_back(std::unique_ptr<TownCargo>(new TownCargo(type, desired, pop_price, sell_order->terminal_id)));

//             const float seller_price = order->get_price();
//             const float buyer_price = pop.get_buy_price(type, seller_price);

//             if (std::isnan(seller_price)) {
//                 ERR_FAIL_MSG("seller price is nan");
//             }
//             if (std::isnan(buyer_price)) {
//                 ERR_FAIL_MSG("buyer_price is nan");
//             }

//             float price = (buyer_price + seller_price) / 2.0f; // Price average

//             if (((price / buyer_price)) > 1.15) { // Too high for buyer no deal
//                 break;
//             }
            
//             if (!order->is_price_acceptable(price)) { // Too low for seller no deal
//                 break;
//             }

//             unsigned int amt = std::min(pop.get_desired(type, price), order->get_amount());
//             if (amt == 0) break; 

//             // get_local_pricer()->report_sale(type, price, amount);
//             // sell_order->sell_cargo(amount, price, money_to_pay); // Calls with money_to_pay
//             // pop.buy_good(type, amount, price);

//             // if (sell_order->amount == 0) {
//             //     sell_it = get_local_pricer()->delete_town_cargo(sell_it);
//             // } else {
//             //     break;
//             // }get_source_id

//             auto res = province->get_capital_and_storage_components_unsafe(order);
//             if (res.first == nullptr || res.second == nullptr) {
//                 ERR_FAIL_MSG("order's owner is invalid");
//             }
//             auto [capital, storage] = res;
//             amt = std::min(amt, (unsigned) floor(storage->get_amount(type)));
//             if (amt == 0) continue;

//             float sub_total = price * amt;

//             capital->add_cash(sub_total);
//             storage->remove_cargo(type, amt);
//             pop.buy_good(type, amt, price);
//         }
//     }
    

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
// }

int32_t MarketComponent::get_current_demand(int type) const {
    int32_t tot = 0;
    if (!last_month_plot.count(type)) return tot;
    for (const auto& [amt, price]: last_month_plot.at(type).first) {
        tot += amt;
    }
    return tot;
}

int32_t MarketComponent::get_current_supply(int type) const {
    int32_t tot = 0;
    if (!last_month_plot.count(type)) return tot;
    for (const auto& [amt, price]: last_month_plot.at(type).second) {
        tot += amt;
    }
    return tot;
}

void MarketComponent::update_last_month_plot() {
    last_month_plot.clear();
    equilibrium_prices.clear();
    
    for (auto& type: get_types_among_orders()) {
        last_month_plot[type] = get_market_info_plot(type);
        equilibrium_prices[type] = find_market_price_equilibrium(type);
    }
}

float MarketComponent::find_market_price_equilibrium(int type) const {
    if (!last_month_plot.count(type)) return 0;
    if (last_month_plot.at(type).first.empty() || last_month_plot.at(type).second.empty()) return 0;

    int demand_included = 0;
    
    const auto& buys  = last_month_plot.at(type).first;
    const auto& sells = last_month_plot.at(type).second;

    int buy_i = 0;
    int sell_i = 0;

    int demand = buys.at(0).first;
    int supply = sells.at(0).first;

    float current_price = 0;

    while (buy_i < buys.size() && sell_i < sells.size()) {
        
        float best_buy_price = buys.at(buy_i).second;
        float best_sell_price = sells.at(sell_i).second;

        if (best_sell_price > best_buy_price) {
            break;
        }

        int traded = std::min(supply, demand);

        supply -= traded;
        demand -= traded;

        current_price = (best_buy_price + best_sell_price) / 2.0f;

        if (supply == 0) {
            sell_i++;
            if (sell_i < sells.size()) {
                supply = sells.at(sell_i).first;
            }
        }
        if (demand == 0) {
            buy_i++;
            if (buy_i < buys.size()) {
                demand = buys.at(buy_i).first;
            }
        }        
    }

    return current_price;
}