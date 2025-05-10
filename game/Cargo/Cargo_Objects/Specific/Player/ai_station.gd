class_name ai_station extends station

const TRADE_MARGINS: float = 1.1
const TIME_UNTIL_RESET_DS_STATION: int = 100

class local_market:
	#Amount available in market
	var input_market: Dictionary[int, int] #Type -> Amount
	#Amount needed in market
	var output_market: Dictionary[int, int] #Type -> Amount
	var local_market_prices: Dictionary[int, float] #Type -> Price
	
	#Represents amount of local goods already commited to request
	var committed_sales: Dictionary[int, int] #Type -> Amount
	
	func _init() -> void:
		input_market = {}
		output_market = {}
		local_market_prices = {}
		committed_sales = {}
		#Inits the market as well
		reset_local_market()
	
	func reset_local_market() -> void:
		for type: int in terminal_map.get_number_of_goods():
			committed_sales[type] = 0
			input_market[type] = 0
			output_market[type] = 0
			local_market_prices[type] = 0.0
	
	func normalize_local_market() -> void:
		#Before it is the price * amount
		for type: int in local_market_prices:
			local_market_prices[type] /= (input_market[type] + output_market[type])
	
	func add_broker_to_market(broker_obj: broker) -> void:
		#Uses orders to allow stations to work, but allows for tampering
		for order: trade_order in broker_obj.get_orders().values():
			var type: int = order.get_type()
			#Used to avoid empty stations messing with ai_stations
			#PBUG: Could break if factories sell everything and have nothing always
			var amount: int = min(order.get_amount(), broker_obj.get_cargo_amount(type))
			if order.is_buy_order():
				output_market[type] += amount
			else:
				input_market[type] += amount
			#Does take max, but makes sure that some margins are guarenteed
			local_market_prices[type] += order.get_limit_price() * amount
	
	func get_price(type: int) -> float:
		return local_market_prices[type]
	
	func get_available(type: int) -> int:
		return input_market[type] - committed_sales[type]
	
	func add_amount_to_committed_sales(type: int, amount: int) -> void:
		committed_sales[type] += amount
	
	func get_committed_sales_for_type(type: int) -> int:
		return committed_sales[type]

var market: local_market

var requests: Dictionary[int, Dictionary] = {} #Type to Dictionary[Vector2i, Array[request]]
var domestic_needs: Dictionary[int, int] = {} #Type -> Amount, Represents amount bought and sold completely domestically

#Represents downstream stations from this station, one for each train that visits here
var ds_stations: Dictionary[int, Vector2i] = {} #AI train id -> downstream station
var timer: Dictionary[int, int] = {} #Set Train_id -> Time until it erases ds_station

func _init(p_location: Vector2i, p_owner: int) -> void:
	super._init(p_location, p_owner)
	market = local_market.new()
	
func get_requests() -> Dictionary[int, Dictionary]:
	return requests

#Will be run every go around to reset timer
func add_ds_station(train_id: int, future_station: Vector2i) -> void:
	if !ds_stations.has(train_id):
		ds_stations[train_id] = future_station
	timer[train_id] = 0

func remove_ds_station(train_id: int) -> void:
	ds_stations.erase(train_id)
	timer.erase(train_id)

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

func fetch_local_market() -> void:
	for tile: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null:
			market.add_broker_to_market(broker_obj)
	market.normalize_local_market()

func update_orders() -> void:
	update_buy_orders()
	update_sell_orders()

func update_buy_orders() -> void:
	#Prioritize Domestic as it should be best rate
	update_domestic_buy_orders()
	#TODO: Order is stuck and could result in bad rates
	for tile: Vector2i in ds_stations.values():
		var ds_station: ai_station = terminal_map.get_ai_station(tile)
		var ds_requests: Dictionary[int, Dictionary] = ds_station.get_requests()
		for type: int in ds_requests:
			for req: request in ds_requests[type].values():
				update_buy_orders_from_request(req)

func update_domestic_buy_orders() -> void:
	var input_market: Dictionary[int, int] = market.input_market
	var output_market: Dictionary[int, int] = market.output_market
	for type: int in input_market:
		if input_market[type] >= output_market[type]:
			#Keep amount needed
			market.add_amount_to_committed_sales(type, output_market[type])
		else:
			#Keep all for domestic
			market.add_amount_to_committed_sales(type, input_market[type])
			#Source rest from request
			update_request(type, location, output_market[type] - input_market[type], market.get_price(type))

func update_buy_orders_from_request(req: request) -> void:
	var type: int = req.type
	#Represents looping request
	if req.source == location:
		return
	
	var amount_avail: int = market.get_available(type)
	
	#Do price checking before seeing if it will fill request or repeat
	if market.get_price(type) * TRADE_MARGINS < req.max_price and amount_avail != 0:
		
		
		var amount_needed_globally: int = 0
		if req.amount > amount_avail:
			market.add_amount_to_committed_sales(type, amount_avail)
			amount_needed_globally = req.amount - amount_avail
		else:
			#More than enough to fill req
			market.add_amount_to_committed_sales(type, req.amount)
		
		#Update orders to amount committed
		update_buy_order(type, market.get_committed_sales_for_type(type), req.max_price)
		
		#Deal with global sourced, pass on
		update_request(type, req.source, amount_needed_globally, req.max_price * TRADE_MARGINS)
	else:
		#If price isn't good then just re-iterate and add margins
		update_request(type, req.source, req.amount, req.max_price * TRADE_MARGINS)

func update_request(p_type: int, p_source: Vector2i, p_amount: int, p_max_price: float) -> void:
	if requests[p_type].has(p_source):
		if p_amount != 0:
			requests[p_type][p_source].amount = p_amount
		else:
			requests[p_type].erase(p_source)
	else:
		requests[p_type][p_source] = request.new(p_type, p_amount, p_source, p_max_price)

func update_buy_order(type: int, amount: int, max_price: float) -> void:
	edit_order(type, amount, true, max_price)

func update_sell_orders() -> void:
	var output_market: Dictionary[int, int] = market.output_market
	for type: int in terminal_map.get_number_of_goods():
		edit_order(type, output_market[type], false, market.get_price(type) * 0.8)
