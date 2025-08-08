#pragma once

#include "firm.hpp"
#include <unordered_map>

namespace godot {

class Hold : public Firm {
    GDCLASS(Hold, Firm);

private:

protected:
    int max_amount;
    static int NUMBER_OF_GOODS;
    static void _bind_methods();

public:
    std::unordered_map<int, float> storage;
    static const int DEFAULT_MAX_STORAGE;

    static Ref<Terminal> create(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    Hold();
    Hold(const Vector2i new_location, const int player_owner, const int p_max_amount);
    virtual void initialize(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    virtual float add_cargo(int type, float amount);
    virtual float get_cargo_amount(int type) const;
    int get_cargo_amount_outside(int type) const;
    virtual void remove_cargo(int type, float amount);
    virtual float transfer_cargo(int type, float amount);
    virtual int transfer_cargo(int type, int amount);
    virtual float get_amount_to_add(int type, float amount) const;

    Dictionary get_current_hold() const;
    void set_current_hold(Dictionary hold);
    float get_current_hold_total() const;
    bool is_full() const;
    bool is_empty() const;

    virtual int get_max_storage() const;
    void change_max_storage(int p_amount);
    void set_max_storage(int p_amount);
    bool does_accept(int type) const;

    static void set_number_of_goods(int p_num);
};

}
