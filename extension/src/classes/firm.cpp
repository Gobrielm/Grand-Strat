#include "firm.hpp"

#include <godot_cpp/core/class_db.hpp>

void Firm::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_amount_can_buy", "amount_per"), &Firm::get_amount_can_buy);
    ClassDB::bind_method(D_METHOD("add_cash"), &Firm::add_cash);
    ClassDB::bind_method(D_METHOD("remove_cash"), &Firm::remove_cash);
    ClassDB::bind_method(D_METHOD("get_cash"), &Firm::get_cash);
    ClassDB::bind_method(D_METHOD("transfer_cash", "amount"), &Firm::transfer_cash);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "location", "owner"), &Firm::create);

    ClassDB::bind_method(D_METHOD("initialize", "location", "owner"), &Firm::initialize);
    
    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::INT, "cash"), "", "get_cash");
    
}

int Firm::get_amount_can_buy(const float amount_per) const {
    return floor(cash / amount_per);
}

void Firm::add_cash(float amount) {
    cash += amount;
}

void Firm::remove_cash(float amount) {
    cash -= amount;
}

float Firm::get_cash() const {
    return cash;
}

float Firm::transfer_cash(float amount) {
    amount = std::min(get_cash(), amount);
	remove_cash(amount);
	return amount;
}
    
Terminal* Firm::create(const Vector2i p_location, const int p_owner) {
     return memnew(Firm(p_location, p_owner));
}

void Firm::initialize(const Vector2i p_location, const int p_owner) {
    Terminal::initialize(p_location, p_owner);
}
