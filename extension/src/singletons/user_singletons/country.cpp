#include "country.hpp"
#include "../money_controller.hpp"
#include "../pop_manager.hpp"

void Country::_bind_methods() {

}

Country::Country(int p_country_id): country_id(p_country_id), minting(DEFAULT_MINTING), player_id(-1) {}

Ref<Country> Country::create_instance() {
    return Ref<Country>(memnew(Country));
}

int Country::get_country_id() const {
    return country_id;
}

void Country::assign_player_id(int p_player_id) {
    player_id = p_player_id;
}

void Country::pay_random_pops(int total_number_to_pay, float total_money_to_pay) {
    double for_each = total_money_to_pay / total_number_to_pay;
    PopManager::get_instance()->pay_pops(total_number_to_pay, total_money_to_pay / total_number_to_pay);
}

void Country::month_tick() {
    if (player_id != -1) {
        pay_random_pops(50, minting);
    }
}

