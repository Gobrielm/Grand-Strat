class_name ai_station extends station

const TRADE_MARGINS: float = 1.1

var stations_in_network: Dictionary[Vector2i, bool] #Set of stations

#TODO: Eventually have ai_trains only take as much as other station needs

func _init(p_location: Vector2i, p_owner: int) -> void:
	super._init(p_location, p_owner)

func month_tick() -> void:
	#Only reset buy orders since system doesn't know if order is cancelled
	reset_buy_orders()
	update_orders()

func reset_buy_orders() -> void:
	for type: int in trade_orders:
		if trade_orders[type].is_buy_order():
			trade_orders.erase(type)

func fetch_local_goods_needed() -> Array[trade_order]:
	var toReturn: Array[trade_order] = []
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			for order: trade_order in broker_obj.get_orders().values():
				if order.is_buy_order():
					toReturn.append(order)
	return toReturn

func update_orders() -> void:
	update_sell_orders()
	update_buy_orders()
	
#Stuff this could sell
func update_buy_orders() -> void:
	# Represents either the max for buying or min for selling
	var market_price: Dictionary[int, float] = create_market_price_for_available_goods() 
	
	for tile: Vector2i in connected_terminals:
		var ai_station_obj: ai_station = terminal_map.get_ai_station(tile)
		if ai_station_obj != null:
			update_buy_orders_for_station(ai_station_obj, market_price)

func update_buy_orders_for_station(ai_station_obj: ai_station, market_price: Dictionary[int, float]) -> void:
	var orders: Dictionary[int, trade_order] = ai_station_obj.get_orders()
	for order: trade_order in orders.values():
		#If other station wants it to sell and will pay higher than min price here
		if order.is_sell_order() and ai_station_obj.get_local_price(order.type) > market_price[order.type]:
			add_amount_to_buy_order(order.type, order.amount, market_price[order.type])

func add_amount_to_buy_order(type: int, amount: int, p_market_price: float) -> void:
	var this_order: trade_order = get_order(type)
	var new_amount: int = amount if !this_order else this_order.amount + amount
	edit_order(type, new_amount, true, p_market_price)

#Stuff this wants
func update_sell_orders() -> void:
	var amount_total: Dictionary[int, int] = {}
	var market_price: Dictionary[int, float] = {} # Represents either the max for buying or min for selling
	for order: trade_order in fetch_local_goods_needed():
		amount_total[order.type] += order.amount
		market_price[order.type] += order.amount * order.max_price
	
	for type: int in amount_total:
		market_price[type] /= amount_total[type]
		edit_order(type, amount_total[type], false, market_price[type])

func create_market_price_for_available_goods() -> Dictionary[int, float]:
	var amount_total: Dictionary[int, int] = {}
	var market_price: Dictionary[int, float] = {} # Represents either the max for buying or min for selling
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			for order: trade_order in broker_obj.get_orders().values():
				if order.is_sell_order():
					amount_total[order.type] += order.amount
					market_price[order.type] += order.amount * order.max_price
	
	for type: int in amount_total:
		market_price[type] /= amount_total[type]
	
	return market_price
