#include <deque>

#include "market_component.hpp"
#include "singletons/terminal_map.hpp"
#include "singletons/data_collector.hpp"
#include "utility/debug_trace.h"
#include "classes/province.hpp"
#include "classes/base_pop.hpp"
#include "classes/components/capital_component.hpp"
#include "classes/components/storage_component.hpp"

MarketComponent::MarketComponent() {
    sale_history.resize(CargoInfo::get_instance()->get_number_of_goods(), std::vector<int>(200, 0));
}

MarketComponent::MarketComponent(const MarketComponent& other): 
    sell_orders(other.sell_orders), 
    buy_orders(other.buy_orders),
    sale_history(other.sale_history),
    equilibrium_prices(other.equilibrium_prices),
    last_month_plot(other.last_month_plot),
    sorted_buy_orders(other.sorted_buy_orders),
    sorted_sell_orders(other.sorted_sell_orders)
{}

std::vector<int> MarketComponent::get_types_among_orders() const {
    std::unordered_set<int> types_added;
    for (auto& [type, _]: sorted_buy_orders) {
        types_added.insert(type);
    }
    for (auto& [type, _]: sorted_sell_orders) {
        types_added.insert(type);
    }
    
    return std::vector<int>(types_added.begin(), types_added.end());
}

void MarketComponent::add_order(std::shared_ptr<TradeOrder> to) {
    if (to->get_price() <= 0.1) {
        print_line(
            "Type:" + String::num(to->get_type()) +
            " Price: " + String::num(to->get_price()) +
            " Limit Price: " + String::num(to->get_limit_price()) +
            " Amount: " + String::num(to->get_amount()) +
            " Owner Type: " + String::num(int(to->get_owner_type())) +
            " Source ID: " + String::num(to->get_source_id())
        );
    }

    if (to->is_buy_order()) {
        buy_orders.push_back(to);
    } else {
        sell_orders.push_back(to);
    }
}

float MarketComponent::get_price(int type) const {
    if (equilibrium_prices.count(type) && equilibrium_prices.at(type) != 0) {
        return equilibrium_prices.at(type);
    }
    return LocalPriceController::get_base_price(type);
}

float MarketComponent::get_min_price(int type) const {
    double min_price = -1;
    if (sorted_buy_orders.count(type) && !sorted_buy_orders.at(type).empty()) {
        min_price = (sorted_buy_orders.at(type).end())->get_price();
    }
    if (sorted_sell_orders.count(type) && !sorted_sell_orders.at(type).empty()) {
        min_price = std::min(min_price, (sorted_sell_orders.at(type).begin())->get_price());
    }

    if (min_price != -1) {
        return min_price;
    }

    return 0.0001;
}

float MarketComponent::get_max_price(int type) const {
    double max_price = -1;
    if (sorted_buy_orders.count(type) && !sorted_buy_orders.at(type).empty()) {
        max_price = (sorted_buy_orders.at(type).begin())->get_price();
    }
    if (sorted_sell_orders.count(type) && !sorted_sell_orders.at(type).empty()) {
        max_price = std::max(max_price, (sorted_sell_orders.at(type).end())->get_price());
    }

    if (max_price != -1) {
        return max_price;
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
            sells.push_back(std::make_pair(order.get_amount(), order.get_price()));
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

Array MarketComponent::get_market_sale_history(int type) const {
    Array toReturn;

    for (int i = 0; i < sale_history[type].size(); i++) {
        toReturn.push_back(sale_history[type][i]);
    }

    return toReturn;
}

const std::vector<int>& MarketComponent::get_market_sale_history_ref(int type) {
    if (sale_history.size() <= type) {
        sale_history.emplace_back();
        ERR_FAIL_V_MSG(sale_history[0], "Sale history is being accessed, before initialized with type: " + String::num(type));
    }
    return sale_history[type];
}

void MarketComponent::market_tick(Province* province) {
    for (auto& [type, buys]: sorted_buy_orders) {
        auto& sell_orders_vec = sorted_sell_orders[type];

        if (sell_orders_vec.empty()) continue;

        while (!buys.empty()) {
            auto& buy_order = buys.front();
            if (buy_order.get_amount() == 0) {
                buys.pop_front();
                continue;
            }

            while (!sell_orders_vec.empty() && (sell_orders_vec.front()).get_amount() == 0) {
                sell_orders_vec.pop_front();
            }
            if (sell_orders_vec.empty()) break;
            auto& sell_order = sell_orders_vec.front();

            float price1 = buy_order.get_price();
            float price2 = sell_order.get_price();
            float avg = (price1 + price2) / 2.0f;

            if (!buy_order.is_price_acceptable(avg) || !sell_order.is_price_acceptable(avg)) {
                break;
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
        if (order->get_price() < 0.0001) {
            print_error("Price Invalid1");
        }
        sorted_buy_orders[type].push_back(*order);
        if (sorted_buy_orders[type].back().get_price() < 0.0001) {
            print_error("Price Invalid2");
        }
    }

    for (auto& order: sell_orders) {
        int type = order->get_type();
        if (order->get_price() < 0.0001) {
            print_error("Price Invalid1");
        }
        sorted_sell_orders[type].push_back(*order);
        if (sorted_sell_orders[type].back().get_price() < 0.0001) {
            print_error("Price Invalid2");
        }
    }

    for (auto& [type, orders]: sorted_buy_orders) {
        std::sort(orders.begin(), orders.end(), TradeOrder::TradeOrderGT());
    }
    for (auto& [type, orders]: sorted_sell_orders) {
        std::sort(orders.begin(), orders.end(), TradeOrder::TradeOrderLT());
    }
}

void MarketComponent::finish_market_exchange(
    TradeOrder& buy_order, 
    TradeOrder& sell_order, 
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

    sell_order.change_amount(sell_order.get_amount() - amt);
    buy_order.change_amount(buy_order.get_amount() - amt);
    
    buyer.first->remove_cash(sub_total);
    seller.first->add_cash(sub_total);

    buyer.second->add_cargo(type, amt);
    seller.second->remove_cargo(type, amt);
    record_sale(type, price, amt);
}

void MarketComponent::sell_to_pop(Province* province, BasePop& pop) {
    // std::unordered_map<int, float> money_to_pay;
    
    for (auto& [type, orders]: sorted_sell_orders) {
        if (pop.get_desired(type) == 0) {
            continue;
        }

        int grain_type = CargoInfo::get_instance()->get_cargo_type("grain");
        if (type == grain_type) {
            DataCollector::get_instance()->add_demand(grain_type, pop.get_desired(type));
        }

        while (!orders.empty()) {
            auto& sell_order = orders.front();
            if (sell_order.get_amount() == 0) {
                orders.pop_front();
                continue;
            }

            // demand_to_relay.push_back(std::unique_ptr<TownCargo>(new TownCargo(type, desired, pop_price, sell_order->terminal_id)));

            const float seller_price = sell_order.get_price();
            const float buyer_price = pop.get_buy_price(type, seller_price);

            
            // DebugTrace::get_instance()->log("Price: " + std::to_string(sell_order.get_price()) + " | Limit: " + std::to_string(sell_order.get_limit_price()));

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
            
            if (!sell_order.is_price_acceptable(price)) { // Too low for seller no deal
                break;
            }

            unsigned int amt = std::min(pop.get_desired(type, price), sell_order.get_amount());
            if (amt == 0) break; 

            // get_local_pricer()->report_sale(type, price, amount);
            // sell_order->sell_cargo(amount, price, money_to_pay); // Calls with money_to_pay
            // pop.buy_good(type, amount, price);

            // if (sell_order->amount == 0) {
            //     sell_it = get_local_pricer()->delete_town_cargo(sell_it);
            // } else {
            //     break;
            // }get_source_id

            auto res = province->get_capital_and_storage_components_unsafe(sell_order);
            if (res.first == nullptr || res.second == nullptr) {
                ERR_FAIL_MSG("order's owner is invalid");
            }
            auto [capital, storage] = res;
            amt = std::min(amt, (unsigned) floor(storage->get_amount(type)));
            if (amt == 0) {
                orders.pop_front();
                continue;
            }

            float sub_total = price * amt;

            capital->add_cash(sub_total);
            storage->remove_cargo(type, amt);
            pop.buy_good(type, amt, price);

            sell_order.change_amount(sell_order.get_amount() - amt);
            record_sale(type, price, amt);

            // TODO: Standardize this more to create pop order
            // TradeOrder buy_order(type, amt, true, )

            // finish_market_exchange(, sell_order)
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

void MarketComponent::bookkeeping_tick() {
    // sort orders
    sort_orders();
    sale_history.resize(CargoInfo::get_instance()->get_number_of_goods(), std::vector<int>(200, 0));

    last_month_plot.clear();
    equilibrium_prices.clear();
    
    for (auto& type: get_types_among_orders()) {
        last_month_plot[type] = get_market_info_plot(type);
        equilibrium_prices[type] = find_market_price_equilibrium(type);
    }
}

float MarketComponent::find_market_price_equilibrium(int type) const {
    
    if (!last_month_plot.count(type) || 
        (last_month_plot.at(type).first.empty() && last_month_plot.at(type).second.empty())) return LocalPriceController::get_base_price(type);

    if (last_month_plot.at(type).second.empty()) return last_month_plot.at(type).first.begin()->second;
    if (last_month_plot.at(type).first.empty()) return last_month_plot.at(type).second.begin()->second;

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
    if (current_price == 0) {
        return LocalPriceController::get_base_price(type);
    }

    return current_price;
}

void MarketComponent::record_sale(int type, double price, int amount) {
    if (price < 0 || price > 99) {
        DebugTrace::get_instance()->log("Record Sale: " + std::to_string(price));
        return;
    }
    sale_history[type][round(price * 2)] += amount;
}