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
        auto pops_data = (pop_manager->get_pop_statistics());

        road_depot_data_points.push_back(terminal_map -> get_average_cash_of_road_depot());
        factory_data_points.push_back(terminal_map -> get_average_cash_of_factory());
        pops_data_points.push_back((*pops_data)[AveragePopWealth]);
        factory_ave_level.push_back(terminal_map -> get_average_factory_level());
        starving_pops.push_back((*pops_data)[NumOfStarvingPops]);
        grain_supply.push_back(0);
        grain_demand.push_back(0);
        broke_pops.push_back((*pops_data)[NumOfBrokePops]);
        unemployement_rate.push_back((*pops_data)[UnemploymentRate]);
        real_unemployement_rate.push_back((*pops_data)[RealUnemploymentRate]);
        number_of_peasants.push_back((*pops_data)[NumOfPeasants]);
        write_data_to_file();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    // print_line("Data Collector Month Tick Took " + String::num_scientific(elapsed.count()) + " seconds");
}

void DataCollector::write_data_to_file() {
    std::ofstream file("data.xlsx");
    file << "Month,";
    file << "Road Depot Data points,";
    file << "Factory Data points,";
    file << "Pops average wealth,";
    file << "Factory average level,";
    file << "Grain Demand,";
    file << "Grain Supply,";
    file << "Starving pops,";
    file << "Broke pops,";
    file << "Unemployment Rate,";
    file << "Real Unemployment Rate,";
    file << "Number Of Peasants";
    file << '\n';
    for (int i = 0; i < road_depot_data_points.size(); i++) {
        file << (i + 1) << ",";
        file << road_depot_data_points[i] << ",";
        file << factory_data_points[i] << ",";
        file << pops_data_points[i] << ",";
        file << factory_ave_level[i] << ",";
        file << grain_demand[i] << ",";
        file << grain_supply[i] << ",";
        file << starving_pops[i] << ",";
        file << broke_pops[i] << ",";
        file << unemployement_rate[i] << ",";
        file << real_unemployement_rate[i] << ",";
        file << number_of_peasants[i];
        file << '\n';
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