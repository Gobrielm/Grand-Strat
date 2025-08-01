#include <iostream>
#include <fstream>
#include "data_collector.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"
#include <godot_cpp/core/class_db.hpp>

DataCollector* DataCollector::singleton_instance = nullptr;

void DataCollector::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &DataCollector::get_instance);

    ClassDB::bind_method(D_METHOD("month_tick"), &DataCollector::month_tick);
}

void DataCollector::_notification(int what) {
    if (what == NOTIFICATION_ENTER_TREE) {
        create();
    }
}

DataCollector::DataCollector() {}
DataCollector::~DataCollector() {}

void DataCollector::create() {
    if (singleton_instance == nullptr) {
        singleton_instance = (memnew(DataCollector));
    }
}
DataCollector* DataCollector::get_instance() {
    return singleton_instance;
}

void DataCollector::month_tick() {
    road_depot_data_points.push_back(TerminalMap::get_instance() -> get_average_cash_of_road_depot());
    factory_data_points.push_back(TerminalMap::get_instance() -> get_average_cash_of_factory());
    pops_data_points.push_back(ProvinceManager::get_instance() -> get_average_cash_of_pops());
    factory_ave_level.push_back(TerminalMap::get_instance() -> get_average_factory_level());
    grain_demand.push_back(TerminalMap::get_instance() -> get_grain_demand());
    grain_supply.push_back(TerminalMap::get_instance() -> get_grain_supply());
    grain_fulfillment.push_back(grain_demand.back() == 0 ? 0: float(grain_supply.back()) / grain_demand.back());
    broke_pops.push_back(ProvinceManager::get_instance() -> get_number_of_broke_pops());
    write_data_to_file();
}

void DataCollector::write_data_to_file() {
    std::ofstream file("data.xlsx");
    file << "Month,";
    for (int i = 1; i < road_depot_data_points.size(); i++) {
        file << i << ", ";
    }
    file << "\nRoad Depot Data points,\n";
    for (float x: road_depot_data_points) {
        file << x;
        file << ",";
    }
    file << "\nFactory Data points,\n";
    for (float x: factory_data_points) {
        file << x;
        file << ",";
    }
    file << "\nPops average wealth,\n";
    for (float x: pops_data_points) {
        file << x;
        file << ",";
    }
    file << "\nFactory average level,\n";
    for (float x: factory_ave_level) {
        file << x;
        file << ",";
    }
    file << "\nGrain Demand,\n";
    for (int x: grain_demand) {
        file << x;
        file << ",";
    }
    file << "\nGrain Supply,\n";
    for (int x: grain_supply) {
        file << x;
        file << ",";
    }
    file << "\nGrain_fulfillment,\n";
    for (float x: grain_fulfillment) {
        file << x;
        file << ",";
    }
    file << "\nBroke_pops,\n";
    for (int x: broke_pops) {
        file << x;
        file << ",";
    }
    file.close();
}