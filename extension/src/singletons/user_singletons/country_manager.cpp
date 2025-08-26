#include "country_manager.hpp"

void CountryManager::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &CountryManager::get_instance);

    ClassDB::bind_method(D_METHOD("create_country", "country_id"), &CountryManager::create_country);
    ClassDB::bind_method(D_METHOD("assign_country_to_player_id", "country_id", "player_id"), &CountryManager::assign_country_to_player_id);
    ClassDB::bind_method(D_METHOD("month_tick"), &CountryManager::month_tick);
}

Ref<CountryManager> CountryManager::singleton_instance = nullptr;

CountryManager::CountryManager() {
}

void CountryManager::create() {
    ERR_FAIL_COND_MSG(singleton_instance != nullptr, "Country Manager has already been created yet.");
    singleton_instance = Ref<CountryManager>(memnew(CountryManager));
}

Ref<CountryManager> CountryManager::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "Country Manager has not been created yet.");
    return singleton_instance;
}

void CountryManager::create_country(int country_id) {
    std::scoped_lock lock(m);
    Ref<Country> country = Ref<Country>(memnew(Country(country_id)));
    countries[country_id] = country; 
}

void CountryManager::assign_country_to_player_id(int country_id, int player_id) {
    std::scoped_lock lock(m);
    countries[country_id] -> assign_player_id(player_id); 
}

void CountryManager::delete_country(int id) {
    std::scoped_lock lock(m);
    countries.erase(id);
}

Ref<Country> CountryManager::get_country(int p_id) const {
    std::scoped_lock lock(m);
    ERR_FAIL_COND_V_MSG(countries.count(p_id) == 0, nullptr, "Country of id" + godot::String::num(p_id) + " does not exist.");
    return countries.at(p_id);
}

void CountryManager::month_tick() {
    std::scoped_lock lock(m);
    for (auto& [__, country]: countries) {
        country->month_tick();
    }
}