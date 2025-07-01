#include "country_manager.hpp"

void CountryManager::_bind_methods() {

}

CountryManager::CountryManager(const Array& peers) {
    for (int i = 0; i < peers.size(); i++) {
        add_country(peers[i]);
    }
}

void CountryManager::create(const Array& peers) {
    ERR_FAIL_COND_MSG(singleton_instance != nullptr, "Country Manager has already been created yet.");
    singleton_instance = Ref<CountryManager>(memnew(CountryManager(peers)));
}

Ref<CountryManager> CountryManager::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "Country Manager has not been created yet.");
    return singleton_instance;
}

void CountryManager::add_country(int new_id) {
    countries[new_id] = Country::create_instance(new_id);
}
void CountryManager::delete_country(int id) {
    countries.erase(id);
}

Ref<Country> CountryManager::get_country(int p_id) const {
    ERR_FAIL_COND_V_MSG(countries.count(p_id) == 0, nullptr, "Country of id" + godot::String::num(p_id) + " does not exist.");
    return countries.at(p_id);
}