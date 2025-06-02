
#include "cargo_info.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

Ref<CargoInfo> CargoInfo::singleton_instance = nullptr;

void CargoInfo::_bind_methods() {
    ClassDB::bind_static_method(CargoInfo::get_class_static(), D_METHOD("get_instance"), &CargoInfo::get_instance);
}

CargoInfo::CargoInfo() {
    ERR_FAIL_COND_MSG(singleton_instance != nullptr, "Cannot create multiple instances of singleton!");
    singleton_instance.instantiate();
    int i = 0;
    for (const auto& [name, __]: base_prices) {
        cargo_names[i] = name;
        cargo_types[name] = i;
        i++;
    }
}

Ref<CargoInfo> CargoInfo::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "Money_Manager has not been created but is being accessed");
    return singleton_instance;
}

const std::unordered_map<int, float> CargoInfo::get_base_prices() {
    std::unordered_map<int, float> toReturn = {};
    for (const auto &[cargo_name, price]: base_prices) {
        toReturn[get_cargo_type(cargo_name)] = price;
    }
    return toReturn;
}

std::string CargoInfo::get_cargo_name(int type) const {
    return cargo_names.at(type);
}

int CargoInfo::get_cargo_type(std::string cargo_name) const {
    if (cargo_types.count(cargo_name) == 0) {
        ERR_FAIL_V_MSG(-1, ("No cargo of name " + cargo_name).c_str());
    }
    return cargo_types.at(cargo_name);
}