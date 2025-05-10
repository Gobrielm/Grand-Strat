class_name ai_station extends station

const TRADE_MARGINS: float = 1.1
const TIME_UNTIL_RESET_DS_STATION: int = 100

class local_market:
	var input_market: Dictionary[int, int] #Type -> Amount
	var output_market: Dictionary[int, int] #Type -> Amount
	var local_market_prices: Dictionary[int, float] #Type -> Price
	
	func _init() -> void:
		#Inits the market as well
		reset_local_market()
	
	func reset_local_market() -> void:
		for type: int in terminal_map.get_number_of_goods():
			input_market[type] = 0
			output_market[type] = 0
			local_market_prices[type] = 0.0
	
	func normalize_local_market() -> void:
		#Before it is the price * amount
		for type: int in local_market_prices:
			local_market_prices[type] /= (input_market[type] + output_market[type])
	
	func add_orders_to_local_market(orders: Dictionary) -> void:
		for order: trade_order in orders.values():
			var type: int = order.get_type()
			var amount: int = order.get_amount()
			if order.is_buy_order():
				output_market[type] += amount
			else:
				input_market[type] += amount
			#Does take max, but makes sure that some margins are guarenteed
			local_market_prices[type] += order.get_limit_price() * amount
	
	func get_price(type: int) -> float:
		return local_market_prices[type]

var market: local_market

var requests: Dictionary[int, Dictionary] #Type to Dictionary[Vector2i, Array[request]]
#Represents amount of local goods already commited to request
var committed_sales: Dictionary[int, int] #Type -> Amount

#Represents downstream stations from this station, one for each train that visits here
var ds_stations: Dictionary[int, Vector2i] #AI train id -> downstream station
var timer: Dictionary[int, int] = {} #Set Train_id -> Time until it erases ds_station

#TODO: Basically sell whatever you have in hold, if price good
#TODO: Buy if it knows it can sell it for higher, or if connected station logic

func _init(p_location: Vector2i, p_owner: int) -> void:
	super._init(p_location, p_owner)
	market = local_market.new()
	
func get_requests() -> Dictionary[int, Dictionary]:
	return requests

func day_tick() -> void:
	super.day_tick()
	for id: int in ds_stations:
		timer[id] += 1
		if timer[id] > TIME_UNTIL_RESET_DS_STATION:
			remove_ds_station(id)

func month_tick() -> void:
	update_all_markets()
	update_orders()

#Thread unsafe
func update_all_markets() -> void:
	market.reset_local_market()
	fetch_local_market()
	update_orders()

func fetch_local_market() -> void:
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			market.add_orders_to_local_market(broker_obj.get_orders())
	market.normalize_local_market()

#Will be run every go around to reset timer
func add_ds_station(train_id: int, future_station: Vector2i) -> void:
	if !ds_stations.has(train_id):
		ds_stations[train_id] = future_station
	timer[train_id] = 0

func remove_ds_station(train_id: int) -> void:
	ds_stations.erase(train_id)
	timer.erase(train_id)

func update_orders() -> void:
	update_buy_orders()
	update_sell_orders()

func update_buy_orders() -> void:
	update_domestic_buy_orders()
	for tile: Vector2i in ds_stations.values():
		var ds_station: ai_station = terminal_map.get_ai_station(tile)
		var ds_requests: Dictionary[int, Dictionary] = ds_station.get_requests()
		for type: int in ds_requests:
			for req: request in ds_requests[type].values():
				update_buy_orders_from_station(req)

func update_domestic_buy_orders() -> void:
	var input_market: Dictionary[int, int] = market.input_market
	var output_market: Dictionary[int, int] = market.output_market
	for type: int in input_market:
		#Make sure inputs needed outweight amount created
		if input_market[type] > output_market[type]:
			#Create request with difference, since the rest can domestic
			update_request(type, location, input_market[type] - output_market[type], market.get_price(type))

func update_buy_orders_from_station(req: request) -> void:
	var type: int = req.type
	#Represents looping request
	if req.source == location:
		return
	#TODO: LOTS OF FIXES AROUND REQUESTS AND BUY/SELLING
	#Do price checking before seeing if it will fill request or repeat
	#if local_market_prices[type] * TRADE_MARGINS < req.max_price:
		#var amount_avail: int = local_market[type] - committed_sales[type]
		#var amount_sourced_globally: int = 0
		#if req.amount > amount_avail:
			#committed_sales[type] = local_market[type]
			#update_buy_order(type, local_market[type], req.max_price)
			#amount_sourced_globally = req.amount - amount_avail
		#else:
			##More than enough to fill req
			#committed_sales[type] += req.amount
			#update_buy_order(type, committed_sales[type], req.max_price)
		#
		##Deal with global sourced, pass on
		#update_request(type, req.source, amount_sourced_globally, req.max_price)
	#else:
		#update_request(type, req.source, req.amount, req.max_price)

func update_request(p_type: int, p_source: Vector2i, p_amount: int, p_max_price: float) -> void:
	if requests[p_type].has(p_source):
		requests[p_type][p_source].amount = p_amount
	else:
		requests[p_type][p_source] = request.new(p_type, p_amount, p_source, p_max_price)

func update_buy_order(type: int, amount: int, max_price: float) -> void:
	edit_order(type, amount, true, max_price)

func update_sell_orders() -> void:
	var output_market: Dictionary[int, int] = market.output_market
	for type: int in output_market:
		update_request(type, location, output_market[type], market.get_price(type))
