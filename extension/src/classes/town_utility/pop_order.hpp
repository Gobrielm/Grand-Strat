#pragma once

struct PopOrder {

    PopOrder(const PopOrder* other);
    PopOrder(int p_type, int p_amount, float p_exp_price, float p_limit_price, int p_pop_id);
    int type;
    int amount;
    float limit_price;
    float exp_price;
    int pop_id;
    int age = 0;


    void buy_cargo(int p_amount);
    

    // Sorts by price
    bool operator>(const PopOrder& other) const;
    bool operator==(const PopOrder& other) const;

    struct PopOrderPtrCompare {
        bool operator()(const PopOrder* lhs, const PopOrder* rhs) const {
            if (lhs->limit_price == rhs->limit_price)
                return lhs < rhs; // fallback to pointer address
            return lhs->limit_price < rhs->limit_price; // highest price first
        }
    };
};