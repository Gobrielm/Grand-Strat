#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <unordered_map>
#include <memory>

using namespace godot;

struct TownCargo {

    TownCargo(const TownCargo* other);
    TownCargo(int p_type, int p_amount, float p_price, int p_terminal_id);
    int type;
    unsigned int amount;
    int age = 0;
    float price;
    int terminal_id;

    std::unordered_map<int, float> fees_to_pay; // Fees are dealt with by the buyer at the very end
    void remove_cargo(int p_amount);
    void sell_cargo(int p_amount);
    void sell_cargo(int p_amount, float p_price);
    void sell_cargo(int p_amount, float p_price, std::unordered_map<int, float>& to_pay);
    void pay_fees(float &total_price);
    void transfer_cargo(int p_amount);
    void return_cargo();
    void return_cargo(std::unordered_map<int, std::unordered_map<int, int>>& cargo_to_return);
    void add_fee_to_pay(int term_id, float fee);

    // Sorts by price
    bool operator>(const TownCargo& other) const;
    bool operator==(const TownCargo& other) const;

    struct TownCargoPtrCompare {
        bool operator()(const std::weak_ptr<TownCargo>& lhs, const std::weak_ptr<TownCargo>& rhs) const {
            auto l = lhs.lock();
            auto r = rhs.lock();
            if (!l && !r) return false; // equal
            if (!l) return true;        // expired < valid
            if (!r) return false;       // valid > expired
            if (l->price == r->price)
                return l.get() < r.get(); // fallback to address
            return l->price < r->price;
        }
    };
};