#pragma once

#include "hold.hpp"
#include <unordered_set>

namespace godot {

class FixedHold : public Hold {
    GDCLASS(FixedHold, Hold);
    

protected:
    static void _bind_methods();
    std::unordered_set<int> accepts;

public:
    
    static Ref<FixedHold> create(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);


    FixedHold();
    FixedHold(const Vector2i new_location, const int player_owner, const int p_max_amount);
    virtual void initialize(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    float add_cargo(int type, float amount) override;
    virtual float add_cargo_ignore_accepts(int type, float amount);
    virtual float transfer_cargo(int type, float amount) override;
    virtual int transfer_cargo(int type, int amount) override;
    virtual int get_desired_cargo(int type) const;

    void reset_accepts();
    Dictionary get_accepts() const;
    virtual std::vector<bool> get_accepts_vector() const;
    void add_accept(int type);
    void remove_accept(int type);
    virtual bool does_accept(int type) const;
    void clear_accepts();
};

}
