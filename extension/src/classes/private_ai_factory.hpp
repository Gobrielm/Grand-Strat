#pragma once

#include "ai_factory.hpp"

using namespace godot;

class PrivateAiFactory : public AiFactory {
    GDCLASS(PrivateAiFactory, AiFactory)
    int cash = 1000;


protected:
    static void _bind_methods();

public:
    PrivateAiFactory();
    ~PrivateAiFactory();
    PrivateAiFactory(Vector2i new_location, Dictionary new_inputs, Dictionary new_outputs);

    static Terminal* create(Vector2i new_location, Dictionary new_inputs, Dictionary new_outputs);

    virtual void initialize(Vector2i new_location, Dictionary new_inputs, Dictionary new_outputs);

    //Orders
    void add_cash(float amount) override;
    void remove_cash(float amount) override;
    float get_cash() const override;
};
