#include "firm.hpp"

#include <godot_cpp/core/class_db.hpp>
#include "../singletons/money_controller.hpp"

void Firm::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "location", "owner"), &Firm::create);

    ClassDB::bind_method(D_METHOD("initialize", "location", "owner"), &Firm::initialize);
    ClassDB::bind_method(D_METHOD("get_amount_can_buy", "amount_per"), &Firm::get_amount_can_buy);
    ClassDB::bind_method(D_METHOD("add_cash"), &Firm::add_cash);
    ClassDB::bind_method(D_METHOD("remove_cash"), &Firm::remove_cash);
    ClassDB::bind_method(D_METHOD("get_cash"), &Firm::get_cash);
    ClassDB::bind_method(D_METHOD("transfer_cash", "amount"), &Firm::transfer_cash);

}

Firm::Firm(): Terminal() {}
Firm::Firm(const Vector2i p_location, const int p_owner): Terminal(p_location, p_owner) {}	

Ref<Terminal> Firm::create(const Vector2i p_location, const int p_owner) {
    return Ref<Terminal>(memnew(Firm(p_location, p_owner)));
}

void Firm::initialize(const Vector2i p_location, const int p_owner) {
    Terminal::initialize(p_location, p_owner);
}

int Firm::get_amount_can_buy(const float amount_per) const {
    return floor(get_cash() / amount_per);
}

void Firm::add_cash(float amount) {
    if (get_player_owner() == 0) {
        std::scoped_lock lock(m);
        cash += amount;
    } else {
        MoneyController::get_instance()->add_money_to_player(get_player_owner(), amount);
    }
}

void Firm::remove_cash(float amount) {
    if (get_player_owner() == 0) {
        std::scoped_lock lock(m);
        cash -= amount;
    } else {
        add_cash(-amount);
    }
}

float Firm::get_cash() const {
    if (get_player_owner() == 0) {
        std::scoped_lock lock(m);
        return cash;
    } else {
        return MoneyController::get_instance()->get_money(get_player_owner());
    }
}

float Firm::get_cash_unsafe() const {
    if (player_owner == 0) {
        return cash;
    } else {
        return MoneyController::get_instance()->get_money(player_owner);
    }
}

float Firm::transfer_cash(float amount) {
    amount = std::min(get_cash(), amount);
	remove_cash(amount);
	return amount;
}
    