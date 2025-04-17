class_name ai_station extends station

var local_market: Dictionary[int, int]
var local_market_prices: Dictionary[int, float]

var global_market: Dictionary[int, int]
var global_market_prices: Dictionary[int, float]

var connected_stations: Dictionary[Vector2i, int]

#TODO: Basically sell whatever you have in hold, if price good
#TODO: Buy if it knows it can sell it for higher, or if connected station logic

func get_local_market() -> Dictionary[int, int]:
	return local_market

func get_local_market_prices() -> Dictionary[int, float]:
	return local_market_prices

func reset_local_market() -> void:
	for type: int in terminal_map.get_number_of_goods():
		local_market[type] = 0
		local_market_prices[type] = 0.0

func reset_global_market() -> void:
	for type: int in terminal_map.get_number_of_goods():
		global_market[type] = 0
		global_market_prices[type] = 0.0

func month_tick() -> void:
	update_all_markets()
	update_orders()

#Thread unsafe
func update_all_markets() -> void:
	reset_local_market()
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			var dist: int = connected_terminals[tile]
			add_orders_to_local_market(broker_obj.get_orders(), dist)
	reset_global_market()
	fetch_global_market()

func add_orders_to_local_market(orders: Dictionary, dist: int) -> void:
	for order: trade_order in orders.values():
		var type: int = order.get_type()
		var amount: int = order.get_amount()
		if dist != 1:
			amount = min(amount, AUTO_ROAD_LOAD_TICK_AMOUNT)
		local_market[type] += amount
		local_market_prices[type] += order.get_max_price() * amount
	
	for type: int in local_market_prices:
		local_market_prices[type] /= local_market[type]

#This function usually will fetch a month old market and sometimes get current market
func fetch_global_market() -> void:
	for tile: Vector2i in connected_stations:
		var station_obj: ai_station = terminal_map.get_ai_station(tile)
		if station_obj != null:
			add_to_global_market(station_obj.get_local_market(), station_obj.get_local_market_prices())
	for type: int in global_market_prices:
		global_market_prices[type] /= global_market[type]

func add_to_global_market(p_local_market: Dictionary[int, int], p_local_market_prices: Dictionary[int, float]) -> void:
	for type: int in p_local_market:
		global_market[type] += p_local_market[type]
		global_market_prices[type] += p_local_market_prices[type]
	
func update_orders() -> void:
	#TODO: Use global market and local market to make orders
	pass
