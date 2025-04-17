class_name ai_station extends station

const TRADE_MARGINS: float = 1.1

var local_market: Dictionary[int, int]
var local_market_prices: Dictionary[int, float]

var global_market: Dictionary[int, int]
var global_market_prices: Dictionary[int, float]

var connected_stations: Dictionary[Vector2i, int] #Set of coords, int represents dist

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
	fetch_local_market()
	reset_global_market()
	fetch_global_market()

func fetch_local_market() -> void:
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			var dist: int = connected_terminals[tile]
			add_orders_to_local_market(broker_obj.get_orders(), dist)
	for type: int in local_market_prices:
		local_market_prices[type] /= local_market[type]

func add_orders_to_local_market(orders: Dictionary, dist: int) -> void:
	add_orders_to_market(orders, dist, true)

#This function usually will fetch a month old market and sometimes get current market
func fetch_global_market() -> void:
	for tile: Vector2i in connected_stations:
		var station_obj: ai_station = terminal_map.get_ai_station(tile)
		if station_obj != null:
			station_obj.get_orders()
			add_orders_to_global_market(station_obj.get_orders(), 1)
	for type: int in global_market_prices:
		global_market_prices[type] /= global_market[type]

func add_orders_to_global_market(orders: Dictionary, dist: int) -> void:
	add_orders_to_market(orders, dist, false)

func add_orders_to_market(orders: Dictionary, dist: int, local: bool) -> void:
	for order: trade_order in orders.values():
		var type: int = order.get_type()
		var amount: int = order.get_amount()
		if dist != 1:
			amount = min(amount, AUTO_ROAD_LOAD_TICK_AMOUNT)
		if local:
			local_market[type] += amount
			local_market_prices[type] += order.get_max_price() * amount
		else:
			global_market[type] += amount
			global_market_prices[type] += order.get_max_price() * amount

#func add_to_global_market(p_local_market: Dictionary[int, int], p_local_market_prices: Dictionary[int, float]) -> void:
	#for type: int in p_local_market:
		#global_market[type] += p_local_market[type]
		#global_market_prices[type] += p_local_market_prices[type]

#TODO: Use global market and local market to make orders
func update_orders() -> void:
	update_buy_orders()

func update_buy_orders() -> void:
	for type: int in local_market:
		var price_mult: float = local_market_prices[type] / global_market_prices[type]
		#Positive Trade, set to take at least 10% margins
		#TODO: Change this number to fit with amount traveled by rail, eg: include expenses
		if price_mult > TRADE_MARGINS:
			update_buy_order(type, global_market[type], global_market_prices[type] / TRADE_MARGINS)

func update_buy_order(type: int, amount: int, max_price: float) -> void:
	edit_order(type, amount, true, max_price)

func update_sell_orders() -> void:
	pass
