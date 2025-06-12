#pragma once

#include <godot_cpp/classes/object.hpp>
#include <unordered_set>
#include "../utility/vector2i_hash.hpp"
#include <vector>

class BasePop;
class Terminal;
class FactoryTemplate;
class Town;

using namespace godot;

class Province : public Object {
    GDCLASS(Province, Object);

    mutable std::mutex m;
    int province_id;
    int country_id = -1;
    int population;
    std::vector<Vector2i> tiles;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> terminal_tiles; //Doesn't 'own' Terminals yet
    std::vector<BasePop*> pops; //Owns pops

    std::vector<Vector2i> get_town_tiles() const;

    protected:
    static void _bind_methods();
    
    

    public:
    //Godot Facing
    static Province* create(int p_prov_id = -1);
    void initialize(int p_prov_id = -1);

    Province();
    Province(int p_prov_id);
    ~Province();


    //Godot Facing
    void add_tile(Vector2i coords);
    int get_population() const;
    void add_population(int population_to_add);
    void set_population(int new_population);
    int get_province_id() const;
    int get_country_id() const;
    void set_country_id(int p_country_id);
    Array get_tiles() const;
    const std::vector<Vector2i>& get_tiles_vector() const;
    Vector2i get_random_tile() const;
    void add_terminal(Vector2i tile);
    void remove_terminal(Vector2i tile);
    Array get_terminal_tiles() const;
    const std::unordered_set<Vector2i, godot_helpers::Vector2iHasher>& get_terminal_tiles_set() const;
    void create_pops();
    int count_pops() const;
    Ref<FactoryTemplate> find_employment(BasePop* pop) const;
    Ref<FactoryTemplate> find_urban_employment(BasePop* pop) const;

    void day_tick();
    void month_tick();
    

    
};