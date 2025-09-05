#pragma once

#include "company_ai.hpp"
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <functional>
#include <memory>
#include <godot_cpp/classes/tile_map_layer.hpp>

class RoadMap;
class TerminalMap;
class ProvinceManager;
class Factory;

enum f_return {
    survey_next,
    do_not_survey_next,
    return_true,
    return_false,
};

// TODO: Current bug with silent crash with either month tick or instansiation

using namespace godot;

class InvestmentCompany : public CompanyAi {
    GDCLASS(InvestmentCompany, CompanyAi);
    static int constexpr MONTHS_OF_CASH_DATA = 12;
    std::deque<float> past_cash;
    const int cargo_type; //Cargo the Company produces

    void record_cash();
    float get_real_gross_profit(int months_to_average) const;
    void pay_employees();
    bool does_have_money_for_investment() override;
    bool should_build() const;
    std::shared_ptr<Vector2i> find_town_for_investment();
    bool does_have_building_in_area_already(const Vector2i& center);
    void build_building(const Vector2i& town_tile);
    std::shared_ptr<Vector2i> find_tile_for_new_building(const Vector2i& town_tile);
    float get_build_score_for_factory(const Vector2i& tile) const;
    
    void build_factory(const Vector2i& factory_tile, const Vector2i& town_tile);
    void build_factory_instantly(const Vector2i& factory_tile, const Vector2i& town_tile);
    void connect_factory(const Vector2i& factory_tile, const Vector2i& town_tile);
    /// @brief Will place the best depot or find the best depot if available
    std::shared_ptr<Vector2i> get_best_depot_tile_or_place_best(const Vector2i& center_tile, const Vector2i& target);
    
    float get_build_score_for_depot(const Vector2i& tile, const Vector2i& target) const;

protected:
    static void _bind_methods();

public:
    static Ref<InvestmentCompany> godot_create(int p_country_id, Vector2i tile, int p_cargo_type);
    static Ref<InvestmentCompany> create(int p_country_id, Vector2i tile, int p_cargo_type);
    static Ref<InvestmentCompany> create(int p_country_id, int p_owner_id, Vector2i tile, int p_cargo_type);

    InvestmentCompany();
    InvestmentCompany(int p_country_id, Vector2i tile, int p_cargo_type);
    InvestmentCompany(int p_country_id, int p_owner_id, Vector2i tile, int p_cargo_type);

    void month_tick() override;

    virtual float get_wage() const;
    int get_cargo_type() const;
    bool is_seeking_investment_from_pops() const;
};

