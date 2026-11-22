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

    auto pop_manager = PopManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    if (is_collecting_data) {
        auto pops_data = (pop_manager->get_pop_statistics());

        subsistence_farm_data_points.push_back(province_manager -> get_average_cash_of_sub_farms());
        factory_data_points.push_back(province_manager -> get_average_cash_of_factory());
        pops_data_points.push_back(pops_data[PopStats::AveragePopWealth]);
        factory_ave_level.push_back(province_manager -> get_average_factory_level());
        starving_pops.push_back(pops_data[PopStats::NumOfStarvingPops]);
        grain_supply.push_back(0);
        grain_demand.push_back(0);
        broke_pops.push_back(pops_data[PopStats::NumOfBrokePops]);
        unemployement_rate.push_back(pops_data[PopStats::UnemploymentRate]);
        real_unemployement_rate.push_back(pops_data[PopStats::RealUnemploymentRate]);
        number_of_peasants.push_back(pops_data[PopStats::NumOfPeasants]);
        grain_price.push_back(province_manager -> get_average_price(CargoInfo::get_instance()->get_cargo_type("grain")));
        write_data_to_file();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    // print_line("Data Collector Month Tick Took " + String::num_scientific(elapsed.count()) + " seconds");
}

void DataCollector::write_data_to_file() {
    std::ofstream file("../data.csv");
    file << "Month,";
    file << "SubsistenceFarms,";
    file << "FactoryDataPoints,";
    file << "PopsAverageWealth,";
    file << "FactoryAverageLevel,";
    file << "GrainDemand,";
    file << "GrainSupply,";
    file << "StarvingPops,";
    file << "BrokePops,";
    file << "UnemploymentRate,";
    file << "Real UnemploymentRate,";
    file << "NumberOfPeasants,";
    file << "PriceOfGrain";
    file << '\n';
    for (int i = 0; i < subsistence_farm_data_points.size(); i++) {
        file << (i + 1) << ",";
        file << subsistence_farm_data_points[i] << ",";
        file << factory_data_points[i] << ",";
        file << pops_data_points[i] << ",";
        file << factory_ave_level[i] << ",";
        file << grain_demand[i] << ",";
        file << grain_supply[i] << ",";
        file << starving_pops[i] << ",";
        file << broke_pops[i] << ",";
        file << unemployement_rate[i] << ",";
        file << real_unemployement_rate[i] << ",";
        file << number_of_peasants[i] << ",";
        file << grain_price[i];
        file << '\n';
    }
    file.close();
}

void DataCollector::add_demand(int type, float amount) {
    std::scoped_lock lock(m);
    if (type == 10) {
        grain_demand.back() += amount;
    }
}

void DataCollector::add_supply(int type, float amount) {
    std::scoped_lock lock(m);
    if (type == 10) {
        grain_supply.back() += amount;
    }
}