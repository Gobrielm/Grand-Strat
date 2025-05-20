class_name ai_station extends station

const TRADE_MARGINS: float = 1.1

var stations_in_network: Dictionary[Vector2i, bool] #Set of stations

#TODO: Eventually have ai_trains only take as much as other station needs

#TODO: Get rid of local pricer, it doesn't work and won;t ever really work

func _init(p_location: Vector2i, p_owner: int) -> void:
	super._init(p_location, p_owner)

func add_station(p_stop: Vector2i) -> void:
	assert(terminal_map.get_ai_station(p_stop) != null)
	stations_in_network[p_stop] = true

func month_tick() -> void:
	update_orders()

func fetch_local_goods_needed() -> Array[trade_order]:
	var toReturn: Array[trade_order] = []
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			for order: trade_order in broker_obj.get_orders().values():
				if order.is_buy_order():
					toReturn.append(order)
	return toReturn

#Returns which types are available
func get_local_goods_available() -> Dictionary[int, bool]:
	var toReturn: Dictionary[int, bool] = {}
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		for order: trade_order in broker_obj.get_orders().values():
			if order.is_sell_order():
				toReturn[order.type] = true
	return toReturn

func update_orders() -> void:
	update_sell_orders()
	update_buy_orders()
	
#Stuff this could sell to trains
func update_buy_orders() -> void:
	var available_goods: Dictionary[int, bool] = get_local_goods_available()
	#Sets all buy orders to amount of 0, but doesn't delete, waits for re-new
	for order: trade_order in trade_orders.values():
		if order.is_buy_order():
			order.amount = 0
	
	
	for tile: Vector2i in stations_in_network:
		var ai_station_obj: ai_station = terminal_map.get_ai_station(tile)
		update_buy_orders_for_station(ai_station_obj, available_goods)
	
	#Cleans up any order still with 0 that weren't re-newed
	clean_up_buy_orders()
	

func update_buy_orders_for_station(ai_station_obj: ai_station, available_goods: Dictionary[int, bool]) -> void:
	var orders: Dictionary[int, trade_order] = ai_station_obj.get_orders()
	for order: trade_order in orders.values():
		#If other station wants it to sell and will pay higher than min price here
		if order.is_sell_order() and available_goods.has(order.type) and ai_station_obj.get_local_price(order.type) > get_local_price(order.type) * TRADE_MARGINS:
			#Order exists and price is not adequete
			add_amount_to_buy_order(order.type, order.amount, get_local_price(order.type) * TRADE_MARGINS)

func add_amount_to_buy_order(type: int, amount: int, p_market_price: float) -> void:
	var this_order: trade_order = get_order(type)
	var new_amount: int = amount if !this_order else this_order.amount + amount
	edit_order(type, new_amount, true, p_market_price)

func clean_up_buy_orders() -> void:
	var to_remove: Array = []
	
	for type: int in trade_orders:
		if trade_orders[type].is_buy_order() and trade_orders[type].amount == 0:
			to_remove.append(type)

	for type: int in to_remove:
		trade_orders.erase(type)


#Stuff this wants from trains
func update_sell_orders() -> void:
	var market: Dictionary[int, trade_order] = create_consolidated_market_for_desired_goods()
	for type: int in market:
		#PBUG:Makes the limit price slightly less than mp, limit_price should be irreleveant
		edit_order(type, market[type].amount, false, market[type].max_price * 0.99)

func create_consolidated_market_for_desired_goods() -> Dictionary[int, trade_order]:
	var amount_total: Dictionary[int, int] = {}
	var market_price: Dictionary[int, float] = {} # Represents either the max for buying or min for selling
	var toReturn : Dictionary[int, trade_order] = {}
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			for order: trade_order in broker_obj.get_orders().values():
				if order.is_buy_order():
					if !amount_total.has(order.type):
						amount_total[order.type] = 0
						market_price[order.type] = 0.0
					amount_total[order.type] += order.amount
					market_price[order.type] += order.amount * broker_obj.get_local_price(order.type)
	
	for type: int in amount_total:
		market_price[type] /= amount_total[type]
		toReturn[type] = trade_order.new(type, amount_total[type], false, market_price[type])
	
	return toReturn

func has_station_connection() -> bool:
	return stations_in_network.size() > 0
