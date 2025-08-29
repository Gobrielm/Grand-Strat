#pragma once

#include "company_ai.hpp"
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <memory>
#include <godot_cpp/classes/tile_map_layer.hpp>

class RoadMap;
class TerminalMap;
class ProvinceManager;
class Factory;

using namespace godot;

class ProspectorAi : public CompanyAi {
    GDCLASS(ProspectorAi, CompanyAi);
    std::unordered_set<int> employees; // Pop ids
    std::vector<int> exisiting_buildings; // terminal_ids
    static int constexpr MONTHS_OF_CASH_DATA = 12;
    std::deque<float> past_cash;
    int cargo_type; //Cargo the Company produces

    void record_cash();
    float get_cash() const;
    float get_real_gross_profit(int months_to_average) const;
    void pay_employees();
    bool does_have_money_for_investment() override;
    void build_building(const Vector2i& town_tile);
    std::shared_ptr<Vector2i> find_town_for_investment();
    bool does_have_building_in_area_already(const Vector2i& center);
    std::shared_ptr<Vector2i> find_tile_for_new_building(const Vector2i& town_tile);
    float get_build_score_for_factory(const Vector2i& tile) const;
    
    void build_factory(const Vector2i& factory_tile, const Vector2i& town_tile);
    void connect_factory(const Vector2i& factory_tile, const Vector2i& town_tile);
    Vector2i* get_best_depot_tile_of_town(const Vector2i& town_tile, const Vector2i& target);
    
    Vector2i* get_best_depot_tile_of_factory(const Vector2i& factory_tile, const Vector2i& target);
    float get_build_score_for_depot(const Vector2i& tile, const Vector2i& target) const;

protected:
    static void _bind_methods();

public:
    
    static ProspectorAi* create(int p_country_id, int p_owner_id, int p_cargo_type);

    ProspectorAi();
    ProspectorAi(int p_country_id, int p_owner_id, int p_cargo_type);
    void employ_pop(int pop_id);

    void month_tick() override;
};

