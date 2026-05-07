#include "storage_component.hpp"
#include "utility/vector2i_hash.hpp"

StorageComponent::StorageComponent(): MAX_STORAGE(INITIAL_MAX_STORAGE) {}

float StorageComponent::get_desired_cargo(int type, float pricePer) {
    return MAX_STORAGE - storage[type];
}

float StorageComponent::get_amount(int type) const {
    if (!storage.count(type)) {
        return 0;
    }
    return storage.at(type);
}

float StorageComponent::add_cargo(int type, float amount) {
    if (amount >= 0) {
        float amt_to_take = std::min(amount, MAX_STORAGE - amount);
        storage[type] += amt_to_take;
        return amount - amt_to_take;
    }
    return amount;
}

void StorageComponent::remove_cargo(int type, float amount) {
    if (amount >= 0) {
        storage[type] -= amount;
        ERR_FAIL_COND_MSG(storage[type] < 0, "Storage went below 0.");
    }
}

void StorageComponent::set_cargo(int type, float amount) {
    storage[type] = amount;
}

std::unordered_map<int, float> StorageComponent::get_storage() {
    return storage;
}

godot::Dictionary StorageComponent::dictionary() const {
   return godot_helpers::convert_map_to_dictionary<int, float>(storage);
}