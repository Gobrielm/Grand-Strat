#include "country.hpp"
#include "../money_controller.hpp"

void Country::_bind_methods() {

}

Country::Country(int p_country_id): country_id(p_country_id), minting(DEFAULT_MINTING) {}

Ref<Country> Country::create_instance() {
    return Ref<Country>(memnew(Country));
}

int Country::get_country_id() const {
    return country_id;
}

void Country::assign_player_id(int p_player_id) {
    player_id = p_player_id;
}

void Country::month_tick() {
    if (player_id != -1) {
        MoneyController::get_instance()->add_money_to_player(player_id, minting);
    }
}

