#include "town_cargo.hpp"
#include "../factory_template.hpp"
#include "../../singletons/terminal_map.hpp"

TownCargo::TownCargo(int p_type, int p_amount, float p_price, Vector2i p_source) {
    type = p_type;
    amount = p_amount;
    price = p_price;
    source = p_source;
}

bool TownCargo::operator>(const TownCargo& other) const {
    return price > other.price;
}

bool TownCargo::operator==(const TownCargo& other) const {
    return price == other.price;
}

void TownCargo::sell_cargo(int p_amount) {
    Ref<FactoryTemplate> factory = TerminalMap::get_instance()->get_terminal_as<FactoryTemplate>(source);
    if (factory.is_null()) return; 
    factory->add_cash(p_amount * price);
    transfer_cargo(p_amount);
}

void TownCargo::transfer_cargo(int p_amount) {
    amount -= p_amount;
}