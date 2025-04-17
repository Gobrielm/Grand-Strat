class_name ai_station extends station

const TRADE_MARGINS: float = 1.1

var local_market: Dictionary[int, int]
var local_market_prices: Dictionary[int, float]

var requests: Dictionary[int, Dictionary] #Type to Dictionary[Vector2i, Array[request]]
#Represents amount of local goods already commited to request
var committed_sales: Dictionary[int, int] #Type -> Amount

#Represents downstream stations from this station
var ds_stations: Dictionary[Vector2i, int] #Set of coords, int represents dist

#TODO: Basically sell whatever you have in hold, if price good
#TODO: Buy if it knows it can sell it for higher, or if connected station logic

func get_local_market() -> Dictionary[int, int]:
	return local_market

func get_local_market_prices() -> Dictionary[int, float]:
	return local_market_prices

func get_requests() -> Dictionary[int, Dictionary]:
	return requests

func reset_local_market() -> void:
	for type: int in terminal_map.get_number_of_goods():
		local_market[type] = 0
		local_market_prices[type] = 0.0

func month_tick() -> void:
	update_all_markets()
	update_orders()

#Thread unsafe
func update_all_markets() -> void:
	reset_local_market()
	fetch_local_market()
	update_orders()

func fetch_local_market() -> void:
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			var dist: int = connected_terminals[tile]
			add_orders_to_local_market(broker_obj.get_orders(), dist)
	for type: int in local_market_prices:
		local_market_prices[type] /= local_market[type]

func add_orders_to_local_market(orders: Dictionary, dist: int) -> void:
	for order: trade_order in orders.values():
		var type: int = order.get_type()
		var amount: int = order.get_amount()
		#If can only trade by road, limit amount
		if dist != 1:
			amount = min(amount, AUTO_ROAD_LOAD_TICK_AMOUNT)
		local_market[type] += amount
		local_market_prices[type] += order.get_max_price() * amount

func update_orders() -> void:
	update_buy_orders()
	update_sell_orders()

func update_buy_orders() -> void:
	for tile: Vector2i in ds_stations:
		var ds_station: ai_station = terminal_map.get_ai_station(tile)
		var ds_requests: Dictionary[int, Dictionary] = ds_station.get_requests()
		for type: int in ds_requests:
			for req: request in ds_requests[type].values():
				update_buy_orders_from_station(req)

func update_buy_orders_from_station(req: request) -> void:
	var type: int = req.type
	#Represents looping request
	if req.source == location:
		return
	
	#Do price checking before seeing if it will fill request or repeat
	if local_market_prices[type] * TRADE_MARGINS < req.max_price:
		var amount_avail: int = local_market[type] - committed_sales[type]
		var amount_sourced_globally: int = 0
		if req.amount > amount_avail:
			committed_sales[type] = local_market[type]
			update_buy_order(type, local_market[type], req.max_price)
			amount_sourced_globally = req.amount - amount_avail
		else:
			#More than enough to fill req
			committed_sales[type] += req.amount
			update_buy_order(type, committed_sales[type], req.max_price)
		
		#Deal with global sourced, pass on
		update_request(type, req.source, amount_sourced_globally, req.max_price)
	else:
		update_request(type, req.source, req.amount, req.max_price)

func update_request(p_type: int, p_source: Vector2i, p_amount: int, p_max_price: float) -> void:
	if requests[p_type].has(p_source):
		requests[p_type][p_source].amount = p_amount
	else:
		requests[p_type][p_source] = request.new(p_type, p_amount, p_source, p_max_price)

func update_buy_order(type: int, amount: int, max_price: float) -> void:
	edit_order(type, amount, true, max_price)

func update_sell_orders() -> void:
	for type: int in local_market:
		update_request(type, location, local_market[type], local_market_prices[type])
