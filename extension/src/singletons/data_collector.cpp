#include <iostream>
#include <fstream>
#include "data_collector.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"
#include "pop_manager.hpp"
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

DataCollector::DataCollector(): is_collecting_data(true) {
    grain_supply.push_back(0);
    grain_demand.push_back(0);
}
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
    auto start_time = std::chrono::high_resolution_clock::now();

    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    auto pop_manager = PopManager::get_instance();
    if (is_collecting_data) {
        road_depot_data_points.push_back(terminal_map -> get_average_cash_of_road_depot());
        factory_data_points.push_back(terminal_map -> get_average_cash_of_factory());
        pops_data_points.push_back(pop_manager -> get_average_cash_of_pops());
        factory_ave_level.push_back(terminal_map -> get_average_factory_level());
        starving_pops.push_back(pop_manager->get_number_of_starving_pops());
        grain_supply.push_back(0);
        grain_demand.push_back(0);
        broke_pops.push_back(pop_manager-> get_number_of_broke_pops());
        unemployement_rate.push_back(pop_manager->get_unemployment_rate());
        real_unemployement_rate.push_back(pop_manager->get_real_unemployment_rate());
        auto pop_type_stats = pop_manager->get_pop_type_statistics();
        number_of_peasants.push_back(pop_type_stats[peasant]);
        write_data_to_file();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    // print_line("Data Collector Month Tick Took " + String::num_scientific(elapsed.count()) + " seconds");
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
    file << "\nReal Unemployment Rate,\n";
    for (float x: real_unemployement_rate) {
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

void DataCollector::add_demand(int type, float amount) {
    std::scoped_lock lock(m);
    if (type == 10)
        grain_demand.back() += amount;
}

void DataCollector::add_supply(int type, float amount) {
    std::scoped_lock lock(m);
    if (type == 10)
        grain_supply.back() += amount;
}