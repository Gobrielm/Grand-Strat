#include "country_instance.hpp"

void Country::_bind_methods() {

}

Country::Country(int p_id): id(p_id) {}

Ref<Country> Country::create_instance(int p_id) {
    return Ref<Country>(memnew(Country(p_id)));
}

// Money Stuff //
void Country::add_money(float amount) {
    money += amount;
}
void Country::remove_money(float amount) {
    money -= amount;
}
float Country::get_money() const {
    return money;
}
float Country::transfer_money(float amount) {
    float toReturn = std::min(amount, get_money());
    remove_money(toReturn);
    return toReturn;
}
bool Country::has_enough_money(int amount) const {
    return get_money() >= amount;
}

void Country::assign_player_id(int p_player_id) {
    player_id = p_player_id;
}