#include "country.hpp"
#include "../money_controller.hpp"
#include "../province_manager.hpp"

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
    Ref<ProvinceManager> prov_manager = ProvinceManager::get_instance();
    int total_pops = prov_manager->get_number_of_pops_in_country(country_id);

    for (const int& province_id: prov_manager->get_country_provinces(country_id)) {
        Province* province = prov_manager->get_province(province_id);
        int pops = province->get_number_of_pops();
        int number_to_pay = pops * total_number_to_pay / total_pops; // Int div
        
        province->pay_pops(number_to_pay, for_each);

        total_pops -= pops;
        total_number_to_pay -= number_to_pay;
    }
}

void Country::month_tick() {
    if (player_id != -1) {
        MoneyController::get_instance()->add_money_to_player(player_id, minting);
        pay_random_pops(1000, minting);
    }
}

