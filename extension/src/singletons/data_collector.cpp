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

DataCollector::DataCollector(): is_collecting_data(true) {}
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
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    if (is_collecting_data) {
        road_depot_data_points.push_back(terminal_map -> get_average_cash_of_road_depot());
        factory_data_points.push_back(terminal_map -> get_average_cash_of_factory());
        pops_data_points.push_back(province_manager -> get_average_cash_of_pops());
        factory_ave_level.push_back(terminal_map -> get_average_factory_level());
        grain_demand.push_back(terminal_map -> get_grain_demand());
        grain_supply.push_back(terminal_map -> get_grain_supply());
        starving_pops.push_back(province_manager->get_number_of_starving_pops());
        broke_pops.push_back(province_manager -> get_number_of_broke_pops());
        unemployement_rate.push_back(province_manager->get_unemployment_rate());
        number_of_peasants.push_back(province_manager->get_number_of_peasants());
        write_data_to_file();
    }
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
    file << "\nStarving pops,\n";
    for (int x: starving_pops) {
        file << x;
        file << ",";
    }
    file << "\nBroke pops,\n";
    for (int x: broke_pops) {
        file << x;
        file << ",";
    }
    file << "\nUnemployment Rate,\n";
    for (float x: unemployement_rate) {
        file << x;
        file << ",";
    }
    file << "\nNumber Of Peasants,\n";
    for (int x: number_of_peasants) {
        file << x;
        file << ",";
    }
    file.close();
}