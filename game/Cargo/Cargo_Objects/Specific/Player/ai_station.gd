class_name ai_station extends station

var local_market: Dictionary[int, int]
var market_prices: Dictionary[int, float]

#TODO: Basically sell whatever you have in hold, if price good
#TODO: Buy if it knows it can sell it for higher, or if connected station logic

func reset_local_market() -> void:
	for type: int in terminal_map.get_number_of_goods():
		local_market[type] = 0
		market_prices[type] = 0.0

func month_tick() -> void:
	reset_local_market()
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			var dist: int = connected_terminals[tile]
			add_orders_to_local_market(broker_obj.get_orders(), dist)

func add_orders_to_local_market(orders: Dictionary, dist: int) -> void:
	for order: trade_order in orders.values():
		var type: int = order.get_type()
		var amount: int = order.get_amount()
		if dist != 1:
			amount = min(amount, AUTO_ROAD_LOAD_TICK_AMOUNT)
		local_market[type] += amount
		market_prices[type] += order.get_max_price() * amount
	
	for type: int in market_prices:
		market_prices[type] /= local_market[type]
