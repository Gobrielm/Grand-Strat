#pragma once
#include "../classes/local_price_controller.hpp"
#include "../singletons/cargo_info.hpp"

class StaticRegistry {
public:
    static void initialize();
    static void uninitialize();
};

