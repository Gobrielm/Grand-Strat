#pragma once

#include <vector>
#include <mutex>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class DataCollector : public Node {
    GDCLASS(DataCollector, Node);

private:
    std::mutex m;
    std::vector<float> road_depot_data_points;
    std::vector<float> factory_data_points;
    std::vector<float> pops_data_points;
    std::vector<float> factory_ave_level;
    std::vector<unsigned long> grain_demand; // Both are collected async but individuals
    std::vector<unsigned long> grain_supply; // Both are collected async but individuals
    std::vector<int> starving_pops;
    std::vector<int> broke_pops;
    std::vector<float> unemployement_rate;
    std::vector<float> real_unemployement_rate;
    std::vector<int> number_of_peasants;
    static DataCollector* singleton_instance;

    bool is_collecting_data;

protected:
    static void _bind_methods();
    void _notification(int what);

public:
    DataCollector();
    ~DataCollector();
    
    static void create();
    static DataCollector* get_instance();

    void month_tick();

    void write_data_to_file();

    void add_demand(int type, float amount);
    void add_supply(int type, float amount);
};
