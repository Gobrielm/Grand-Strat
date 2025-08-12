#include "trade_interaction.hpp"

TradeInteraction::TradeInteraction(float p_price, Ref<Broker> p_main_buyer, Ref<RoadDepot> p_middleman) {
    price = p_price;
    main_buyer = p_main_buyer;
    middleman = p_middleman;
}