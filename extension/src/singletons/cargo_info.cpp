#include "cargo_info.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

Ref<CargoInfo> CargoInfo::singleton_instance = Ref<CargoInfo>(nullptr);

void CargoInfo::_bind_methods() {
    ClassDB::bind_static_method(CargoInfo::get_class_static(), D_METHOD("get_instance"), &CargoInfo::get_instance);

    ClassDB::bind_method(D_METHOD("get_cargo_array"), &CargoInfo::get_cargo_array);
    ClassDB::bind_method(D_METHOD("get_cargo_name", "type"), &CargoInfo::get_cargo_name);
    ClassDB::bind_method(D_METHOD("get_cargo_type", "cargo_name"), &CargoInfo::get_cargo_type);
    ClassDB::bind_method(D_METHOD("get_amount_of_primary_goods"), &CargoInfo::get_amount_of_primary_goods);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "amount_of_primary_goods"), "", "get_amount_of_primary_goods");
}

CargoInfo::CargoInfo() {
    int i = 0;
    for (const String s: cargo_names) {
        cargo_types[s.utf8().get_data()] = i;
        i++;
    }
    create_amount_of_primary_goods();
}

CargoInfo::~CargoInfo() {
}

void CargoInfo::initialize_singleton() {
    if (singleton_instance.is_null()) {
        singleton_instance.instantiate();
    }   
}

void CargoInfo::cleanup() {
    if (singleton_instance.is_valid()) {
        singleton_instance.unref();
    }
}

Ref<CargoInfo> CargoInfo::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance.is_null(), Ref<CargoInfo>(nullptr), "CargoInfo has not been created but is being accessed.");
    return singleton_instance;
}

std::unordered_map<int, float> CargoInfo::get_base_prices() const {
    std::unordered_map<int, float> toReturn = {};
    for (const auto &[cargo_name, price]: base_prices) {
        toReturn[get_cargo_type(cargo_name.c_str())] = price;
    }
    return toReturn;
}

String CargoInfo::get_cargo_name(int type) const {
    return cargo_names.at(type);
}

int CargoInfo::get_cargo_type(String cargo_name) const {
    if (cargo_types.count(cargo_name.utf8().get_data()) == 0) {
        ERR_FAIL_V_MSG(-1, ("No cargo of name " + cargo_name));
    }
    return cargo_types.at(cargo_name.utf8().get_data());
}

void CargoInfo::create_amount_of_primary_goods() {
    for (int i = 0; i < cargo_types.size(); ++i) {
        if (String(cargo_names[i]) == "gold") {
            amount_of_primary_goods = i + 1;
            break;
        }
    }
}

int CargoInfo::get_amount_of_primary_goods() const {
    return amount_of_primary_goods;
}

int CargoInfo::get_number_of_goods() const {
    return cargo_names.size();
}

bool CargoInfo::is_cargo_primary(int cargo_type) const {
    return cargo_type < amount_of_primary_goods;
}

Array CargoInfo::get_cargo_array() const {
    Array toReturn;
    for (String x: cargo_names) {
        toReturn.push_back(x);
    }
    return toReturn;
}