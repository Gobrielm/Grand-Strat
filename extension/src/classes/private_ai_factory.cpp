
#include "private_ai_factory.hpp"

void PrivateAiFactory::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "new_inputs", "new_outputs"), &PrivateAiFactory::create);
    ClassDB::bind_method(D_METHOD("add_cash", "amount"), &PrivateAiFactory::add_cash);
    ClassDB::bind_method(D_METHOD("remove_cash", "amount"), &PrivateAiFactory::remove_cash);
    ClassDB::bind_method(D_METHOD("get_cash"), &PrivateAiFactory::get_cash);
}

PrivateAiFactory::PrivateAiFactory(): AiFactory() {}

PrivateAiFactory::~PrivateAiFactory() {}

PrivateAiFactory::PrivateAiFactory(Vector2i new_location, Dictionary new_inputs, Dictionary new_outputs) {
    AiFactory::initialize(new_location, 0, new_inputs, new_outputs);
}

Terminal* PrivateAiFactory::create(Vector2i new_location, Dictionary new_inputs, Dictionary new_outputs) {
    return memnew(PrivateAiFactory(new_location, new_inputs, new_outputs));
}

void PrivateAiFactory::initialize(Vector2i new_location, Dictionary new_inputs, Dictionary new_outputs) {
    AiFactory::initialize(new_location, 0, new_inputs, new_outputs);
}

//Orders
void PrivateAiFactory::add_cash(float amount) {
    amount += amount;
}
void PrivateAiFactory::remove_cash(float amount) {
    cash -= amount;
}
float PrivateAiFactory::get_cash() const {
    return cash;
}