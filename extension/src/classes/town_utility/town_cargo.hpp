#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/classes/weak_ref.hpp>

using namespace godot;

struct TownCargo {

    TownCargo(const TownCargo* other);
    TownCargo(int p_type, int p_amount, float p_price, int p_terminal_id);
    int type;
    int amount;
    int age = 0;
    float price;
    int terminal_id;

    std::unordered_map<int, float> fees_to_pay;

    void sell_cargo(int p_amount);
    void sell_cargo(int p_amount, float p_price);
    void sell_cargo(int p_amount, float p_price, std::unordered_map<int, float>& to_pay);
    void pay_fees(float &total_price);
    void transfer_cargo(int p_amount);
    void return_cargo();
    void add_fee_to_pay(int term_id, float fee);

    // Sorts by price
    bool operator>(const TownCargo& other) const;
    bool operator==(const TownCargo& other) const;

    struct TownCargoPtrCompare {
        bool operator()(const TownCargo* lhs, const TownCargo* rhs) const {
            if (lhs->price == rhs->price)
                return lhs < rhs; // fallback to pointer address
            return lhs->price > rhs->price; // lowest price first
        }
    };
};