#pragma once

#include "hold.hpp"
#include <unordered_set>

namespace godot {

class FixedHold : public Hold {
    GDCLASS(FixedHold, Hold);

    std::unordered_set<int> accepts;

protected:
    static void _bind_methods();

public:
    static Terminal* create(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    FixedHold();
    FixedHold(const Vector2i new_location, const int player_owner, const int p_max_amount);
    virtual void initialize(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    int add_cargo(int type, int amount);
    int add_cargo_ignore_accepts(int type, int amount);
    int transfer_cargo(int type, int amount);
    virtual int get_desired_cargo(int type) const;

    void reset_accepts();
    Dictionary get_accepts() const;
    void add_accept(int type);
    void remove_accept(int type);
    bool does_accept(int type) const;
};

}
