#pragma once

#include <unordered_map>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class MoneyController : public RefCounted {
    GDCLASS(MoneyController, RefCounted);

private:
    std::unordered_map<int, float> money;
    static Ref<MoneyController> singleton_instance;
    static const int INITIAL_AMOUNT_OF_MONEY;

protected:
    static void _bind_methods();

public:
    MoneyController();
    
    static void create(const Array& peers);
    static Ref<MoneyController> get_instance();

    void add_peer(int new_id);
    void delete_peer(int id);

    void add_money_to_player(int id, float amount);
    void remove_money_from_player(int id, float amount);

    float get_money(int id) const;
    Dictionary get_money_dictionary() const;
    bool player_has_enough_money(int id, int amount) const;
};
