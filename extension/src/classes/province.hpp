#pragma once

#include <godot_cpp/classes/object.hpp>
#include "base_pop.hpp"
#include "rural_pop.hpp"
#include "town_pop.hpp"
#include "terminal.hpp"
#include "factory_template.hpp"
#include "town.hpp"
#include <unordered_map>
#include <vector>

using namespace godot;

class Province : public Object {
    GDCLASS(Province, Object);

    int province_id;
    int country_id = -1;
    int population;
    std::vector<Vector2i> tiles;
    std::unordered_map<Vector2i, Terminal*> terminal_tiles; //Doesn't 'own' Terminals yet
    std::vector<BasePop*> pops; //Owns pops

    std::vector<Town*> get_towns() const;

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
    void set_country_id(int p_country_id);
    Array get_tiles() const;
    Vector2i get_random_tile() const;
    void add_terminal(Vector2i tile, Terminal* term);
    void remove_terminal(Vector2i tile);
    Array get_terminals() const;
    void create_pops();
    int count_pops() const;
    FactoryTemplate* find_employment(BasePop* pop) const;
    bool will_work_here(BasePop* pop, FactoryTemplate* fact) const;

   
    

    
};