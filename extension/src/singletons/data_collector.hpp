#pragma once

#include <vector>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

class DataCollector : public Node {
    GDCLASS(DataCollector, Node);

private:
    std::vector<float> road_depot_data_points;
    std::vector<float> factory_data_points;
    std::vector<float> pops_data_points;
    std::vector<float> factory_ave_level;
    std::vector<int> grain_demand;
    std::vector<int> grain_supply;
    std::vector<float> grain_fulfillment;
    std::vector<int> broke_pops;
    static DataCollector* singleton_instance;

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
};
