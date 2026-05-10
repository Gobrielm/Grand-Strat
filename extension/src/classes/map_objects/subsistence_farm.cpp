#include "subsistence_farm.hpp"
#include "town.hpp"

#include "singletons/recipe_info.hpp"
#include "singletons/cargo_info.hpp"
#include "singletons/data_collector.hpp"

SubsistenceFarm::SubsistenceFarm(): employer() {
    RecipeInfo::create_employer_component(
        {{}, 
        {{"grain", 11.0f}}}, 
        {{PopTypes::peasant, 10}}
    );
}

SubsistenceFarm::SubsistenceFarm(Vector2i p_location, int p_owner): position(std::make_pair(p_location.x, p_location.y), BuildingType::SUBSISTENCE_FARM), owner(p_owner) {
    employer = RecipeInfo::create_employer_component(
        {{}, 
        {{"grain", 11.0f}}}, 
        {{PopTypes::peasant, 10}}
    );
}

SubsistenceFarm::SubsistenceFarm(const SubsistenceFarm& other): position(other.position), owner(other.owner), storage(other.storage), capital(other.capital), employer(other.employer), orders(other.orders) {}

SubsistenceFarm& SubsistenceFarm::operator=(const SubsistenceFarm& other) {
    if (this == &other) return *this;
    
    position = other.position;
    storage = other.storage;
    owner = other.owner;
    capital = other.capital;
    employer = other.employer;
    orders = other.orders;

    return *this;
}

void SubsistenceFarm::add_pop(Town& town, BasePop* pop) {
    employer.add_pop(pop);
    
    pop->employ(position.get_building_id(), employer.get_wage(town, capital.get_cash()));
    pop->set_location(position.get_position_vector2i());
    consider_upgrade();
}

void SubsistenceFarm::adjust_trade_orders(Town& town) {

    auto& recipe = employer.recipe;

    for (const auto& [type, amt]: recipe.get_outputs()) {
        if (!orders.count(type)) {
            orders[type] = std::make_shared<TradeOrder>(position, type, amt, false, town.mp.get_price(type), town.mp.get_min_price(type));
            town.mp.add_order(orders[type]);
        }

        orders[type]->change_amount(storage.get_amount(type));
        orders[type]->set_max_price(town.mp.get_min_price(type));
        orders[type]->set_price(town.mp.get_price(type));
    }
}

double SubsistenceFarm::get_batch_size() {
    double batch_size = employer.get_level();
    for (auto& [type, amount]: employer.recipe.get_outputs()) {
        batch_size = std::min((double(storage.MAX_STORAGE) - storage.get_amount(type)) / amount, batch_size);
    }
    return batch_size;
}

void SubsistenceFarm::create_recipe() {
    double batch_size = get_batch_size();
    if (batch_size == 0) return;
    for (const auto& [type, amount]: employer.recipe.get_outputs()) {
        storage.add_cargo(type, amount * batch_size);
        DataCollector::get_instance()->add_supply(type, amount * batch_size);
    }
}

void SubsistenceFarm::month_tick() {
    create_recipe();
    consider_degrade();
    consider_upgrade();
}

EmployerComponent SubsistenceFarm::get_default_employer_component() {
    return RecipeInfo::create_employer_component(
        {{}, 
        {{"grain", 11.0f}}}, 
        {{PopTypes::peasant, 10}}
    );
}

void SubsistenceFarm::consider_upgrade() {
    if (employer.get_employment_rate() > 0.8) {
        employer.upgrade();
    }
}

void SubsistenceFarm::consider_degrade() {
    if (employer.get_employment_rate() < 0.5 && employer.get_level_without_employment() > 1) {
        employer.degrade();
    }
}

