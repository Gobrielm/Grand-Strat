#pragma once

#include "firm.hpp"
#include <unordered_map>

namespace godot {

class Hold : public Firm {
    GDCLASS(Hold, Firm);

private:
    int max_amount;
    static int NUMBER_OF_GOODS;

protected:
    static void _bind_methods();

public:
    std::unordered_map<int, int> storage;
    static const int DEFAULT_MAX_STORAGE;

    static Terminal* create(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    Hold();
    Hold(const Vector2i new_location, const int player_owner, const int p_max_amount);
    virtual void initialize(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    virtual int add_cargo(int type, int amount);
    int get_cargo_amount(int type) const;
    virtual void remove_cargo(int type, int amount);
    int transfer_cargo(int type, int amount);
    virtual int get_amount_to_add(int type, int amount) const;

    Dictionary get_current_hold() const;
    void set_current_hold(Dictionary hold);
    int get_current_hold_total() const;
    bool is_full() const;
    bool is_empty() const;

    int get_max_storage() const;
    void change_max_storage(int p_amount);
    void set_max_storage(int p_amount);
    bool does_accept(int type) const;

    static void set_number_of_goods(int p_num);
};

}
