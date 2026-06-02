#include "subsistence_farm.hpp"
#include "town.hpp"

#include "singletons/recipe_info.hpp"
#include "singletons/cargo_info.hpp"
#include "singletons/data_collector.hpp"

SubsistenceFarm::SubsistenceFarm(): employer() {}

SubsistenceFarm::SubsistenceFarm(Vector2i p_location, int p_owner): position(std::make_pair(p_location.x, p_location.y), BuildingType::SUBSISTENCE_FARM), owner(p_owner) {
    employer = RecipeInfo::create_employer_component(
        {{}, 
        {{"grain", 11.0f}}}, 
        {{PopTypes::peasant, 10}}
    );
    storage_delta_indicator = std::vector<int>(CargoInfo::get_instance()->get_number_of_goods(), 0);
}

SubsistenceFarm::SubsistenceFarm(const SubsistenceFarm& other): 
    position(other.position), 
    owner(other.owner), 
    storage(other.storage), 
    last_month_storage(other.last_month_storage), 
    capital(other.capital), 
    employer(other.employer), 
    orders(other.orders),
    storage_delta_indicator(other.storage_delta_indicator)
    {}

SubsistenceFarm& SubsistenceFarm::operator=(const SubsistenceFarm& other) {
    if (this == &other) return *this;
    
    position = other.position;
    storage = other.storage;
    last_month_storage = other.last_month_storage;
    owner = other.owner;
    capital = other.capital;
    employer = other.employer;
    orders = other.orders;
    storage_delta_indicator = other.storage_delta_indicator;

    return *this;
}

void SubsistenceFarm::add_pop(Town& town, BasePop* pop) {
    employer.add_pop(pop);
    
    pop->employ(position.get_building_id(), employer.get_wage(town, capital.get_cash()));
    pop->set_location(position.get_position_vector2i());
    consider_upgrade();
}

float SubsistenceFarm::get_wage() {
    if (employer.get_employement() == 0) return 0;
    return capital.get_cash() / employer.get_employement();
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
        float amt = amount * batch_size;
        storage.add_cargo(type, amt);
        DataCollector::get_instance()->add_supply(type, amt);
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

// void SubsistenceFarm::adjust_trade_orders(Town& town) {

//     auto& recipe = employer.recipe;

//     for (const auto& [type, amt]: recipe.get_outputs()) {
//         float town_price = town.mp.get_price(type);
//         if (town_price == 0) town_price = 0.1;

//         if (!orders.count(type)) {
//             orders[type] = std::make_shared<TradeOrder>(position, type, amt, false, town_price, 0.1); // Arbitrary limit price
//             town.mp.add_order(orders[type]);
//         }

//         orders[type]->change_amount(storage.get_amount(type));
//         orders[type]->set_price(town_price);
//     }
// }

void SubsistenceFarm::adjust_trade_orders(Town& town) {

    // diff = actual - wanted
    auto getPriceMult = [this] (int type, int diff) {
        double toReturn = 1.0;
        if (diff > 0) {

            if (storage_delta_indicator[type] > 3) {
                toReturn = 0.99;
            }

            storage_delta_indicator[type] = std::min(storage_delta_indicator[type] + 1, 5);
        } else {

            if (storage_delta_indicator[type] < -3) {
                toReturn = 1.01;
            }

            storage_delta_indicator[type] = std::max(storage_delta_indicator[type] - 1, -5);
        }
        return toReturn;
    };

    auto& recipe = employer.recipe;
    
    for (const auto& [type, amt]: recipe.get_outputs()) {
        if (!orders.count(type)) {
            orders[type] = std::make_shared<TradeOrder>(position, type, amt, false, town.mp.get_price(type), 0.1); // arbitrary price
            town.mp.add_order(orders[type]);
        }

        float price = orders[type]->get_price();

        int diff = last_month_storage.get_amount(type) - storage.get_amount(type);
        float mult = getPriceMult(type, diff);

        float new_price = std::max(0.1f, price * mult); // arbitrayt price
        orders[type]->set_price(new_price);
    }
    last_month_storage = storage;
}