#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/array.hpp>

#include <singletons/cargo_info.hpp>
#include <memory>

using namespace godot;



enum class TradeOrderOwner: int {
    INVALID = -1,
    POP = 1,
    BUILDING = 2,
    COMPANY = 3, // If changing here, change get_order_id()
};

class TradeOrder {

    private:
    int type = 0;
    int amount = 0;
    bool buy = true;

    double price = 0.0;
    double limit_price = 0.0;

    int source_id = -1;
    TradeOrderOwner owner_type = TradeOrderOwner::INVALID;
    bool active = true;
    

    void initialize(int p_type, int p_amount, bool p_buy, double p_limit_price);

    // Assumes, one order of type from owner
    int32_t get_order_id() const {
        int32_t x = (source_id * CargoInfo::get_instance()->get_number_of_goods() * 4 + type * 4 + int(owner_type));
        if (owner_type == TradeOrderOwner::INVALID) {
            print_error("Getting order id from invalid owner");
        }
        return x;
    }

public:


    TradeOrder();
    TradeOrder(int p_type, int p_amount, bool p_buy, double p_limit_price);
    TradeOrder(class PositionComponent pos, int p_type, int p_amount, bool p_buy, double p_price, double p_limit_price);
    TradeOrder(int p_source_id, TradeOrderOwner p_owner_type, int p_type, int p_amount, bool b_buy, double p_price, double p_limit_price);

    struct TradeOrderGT {
        bool operator()(const std::shared_ptr<TradeOrder> order1, const std::shared_ptr<TradeOrder> order2) const {
            if (order1->price == order2->price) {
                return order1->get_order_id() < order2->get_order_id();
            }
            return order1->price > order2->price;
        }

        bool operator()(const TradeOrder& order1, const TradeOrder& order2) const {
            if (order1.price == order2.price) {
                return order1.get_order_id() < order2.get_order_id();
            }
            return order1.price > order2.price;
        }
    };

    struct TradeOrderLT {
        bool operator()(const std::shared_ptr<TradeOrder> order1, const std::shared_ptr<TradeOrder> order2) const {
            if (order1->price == order2->price) {
                return order1->get_order_id() < order2->get_order_id();
            }
            return order1->price < order2->price;
        }

        bool operator()(const TradeOrder& order1, const TradeOrder& order2) const {
            if (order1.price == order2.price) {
                return order1.get_order_id() < order2.get_order_id();
            }
            return order1.price < order2.price;
        }
    };

    static TradeOrder* create(int p_type, int p_amount, bool p_buy, double p_limit_price);

    static TradeOrder* create_buy_order(int p_type, int p_amount, double p_limit_price);
    static TradeOrder* create_sell_order(int p_type, int p_amount, double p_limit_price);

    bool is_buy_order() const;
    bool is_sell_order() const;
    void change_buy(bool _buy);

    int get_type() const;
    void change_amount(int p_amount);
    unsigned int get_amount() const;

    double get_price() const;
    double get_limit_price() const;
    void set_max_price(double p_max_price);

    void set_price(double p_price);

    TradeOrderOwner get_owner_type() const;

    Array convert_to_array() const;
    bool is_price_acceptable(double price) const;

    int get_source_id() const;

    static TradeOrder* construct_from_array(const Array& array);

    void cancel_order();
};