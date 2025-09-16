#pragma once
#include "../broker.hpp"
#include "../road_depot.hpp"

struct TradeInteraction {
    Ref<Broker> main_buyer;
    Ref<RoadDepot> middleman;
    float price;

    TradeInteraction(float p_price = 0.0, Ref<Broker> p_main_buyer = Ref<Broker>(nullptr), Ref<RoadDepot> p_middleman = Ref<RoadDepot>(nullptr));
};