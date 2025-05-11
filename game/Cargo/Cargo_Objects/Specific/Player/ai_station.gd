class_name ai_station extends station

const TRADE_MARGINS: float = 1.1

var stations_in_network: Dictionary[Vector2i, bool] #Set of stations

#TODO: Eventually have ai_trains only take as much as other station needs

func _init(p_location: Vector2i, p_owner: int) -> void:
	super._init(p_location, p_owner)

func month_tick() -> void:
	#Only reset buy orders since they will change daristically
	reset_buy_orders()
	update_orders()

func reset_buy_orders() -> void:
	for type: int in trade_orders:
		if trade_orders[type].is_buy_order():
			trade_orders.erase(type)

func fetch_local_goods_available() -> Array[trade_order]:
	return fetch_local_orders(false)

func fetch_local_goods_needed() -> Array[trade_order]:
	return fetch_local_orders(true)

func fetch_local_orders(needed: bool) -> Array[trade_order]:
	#TODO: Sort by max_price
	var toReturn: Array[trade_order] = []
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			for order: trade_order in broker_obj.get_orders().values():
				if (order.is_sell_order() and !needed) or (order.is_buy_order() and needed):
					toReturn.append(order)
	return toReturn
	

func update_orders() -> void:
	update_sell_orders()
	update_buy_orders()
	
#Stuff this could sell
func update_buy_orders() -> void:
	var amount_total: Dictionary[int, int] = {}
	var market_price: Dictionary[int, float] = {}
	for order: trade_order in fetch_local_goods_available():
		amount_total[order.type] += order.amount
		market_price[order.type] += order.amount * order.max_price
	
	for type: int in amount_total:
		market_price[type] /= amount_total[type]
	
	for tile: Vector2i in connected_terminals:
		var ai_station_obj: ai_station = terminal_map.get_ai_station(tile)
		if ai_station_obj == null:
			continue
		var orders: Dictionary[int, trade_order] = ai_station_obj.get_orders()
		for order: trade_order in orders.values():
			if order.is_buy_order():
				continue
			var this_order: trade_order = get_order(order.type)
			var this_amount: int = 0
			if this_order:
				#Current amount already wanted
				this_amount = this_order.amount
			#IF station.local_price(order.type) > market_price[order.type]:
			#	edit_order
			if order.max_price < market_price[order.type]:
				edit_order(order.type, this_order.amount + this_amount, true,  market_price[order.type])

#Stuff this wants
func update_sell_orders() -> void:
	var amount_total: Dictionary[int, int] = {}
	var market_price: Dictionary[int, float] = {}
	for order: trade_order in fetch_local_goods_needed():
		amount_total[order.type] += order.amount
		market_price[order.type] += order.amount * order.max_price
	
	for type: int in amount_total:
		market_price[type] /= amount_total[type]
		edit_order(type, amount_total[type], false, market_price[type])
		
