#pragma once

struct PopOrder {

    static constexpr float MAX_DIFF = 0.1;
    PopOrder(const PopOrder* other);
    PopOrder(int p_type, int p_amount, float p_price, int p_pop_id);
    int type;
    int amount;
    float price;
    int pop_id;
    int age = 0;

    void buy_cargo(int p_amount);

    // Sorts by price
    bool operator>(const PopOrder& other) const;
    bool operator==(const PopOrder& other) const;

    struct PopOrderPtrCompare {
        bool operator()(const PopOrder* lhs, const PopOrder* rhs) const {
            if (lhs->price == rhs->price)
                return lhs < rhs; // fallback to pointer address
            return lhs->price < rhs->price; // highest price first
        }
    };
};