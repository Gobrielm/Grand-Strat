class_name town extends broker

var internal_factories: Dictionary[int, Array] = {} # Owner id -> Array[factory_templates]
var city_pops: Dictionary[int, city_pop] = {} #Pop id -> pop
var market: town_market

func _init(new_location: Vector2i) -> void:
	super._init(new_location, 0) #Inits with 0 pops, and player_id = 0
	market = town_market.new()

# === Trade ===

func does_accept(type: int) -> bool:
	return storage[type] != max_amount

func get_local_price(type: int) -> float:
	return market.get_local_price(type)

func is_price_acceptable(type: int, price: float) -> bool:
	return market.is_price_acceptable(type, price)

func get_desired_cargo_to_load(type: int, price: float) -> int:
	return market.get_desired_cargo_to_load(type, price)

func buy_cargo(type: int, amount: int, price: float) -> void:
	market.buy_cargo(type, amount, price)

#Returns with the amount of cargo sold
func sell_cargo(type: int, amount: int, price: float) -> int:
	return market.sell_cargo(type, amount, price)

func get_fulfillment(type: int) -> float:
	return market.get_fulfillment(type)

func get_fulfillment_dict() -> Dictionary[int, float]:
	var toReturn: Dictionary[int, float] = {}
	for type: int in market.supply:
		toReturn[type] = get_fulfillment(type)
	return toReturn

# === Factories ===
func add_factory(fact: factory_template) -> void:
	if !internal_factories.has(fact.get_player_owner()):
		internal_factories[fact.get_player_owner()] = []
	internal_factories[fact.get_player_owner()].append(internal_factories)
	fact.add_connected_terminal(self)

# === Pops ===
func add_pop(pop: base_pop) -> void:
	city_pops[pop.pop_id] = pop

func get_pops() -> Array[base_pop]:
	var toReturn: Array[base_pop]
	toReturn.assign(city_pops.values())
	return toReturn

func sell_to_pops() -> void:
	for type: int in market.get_supply():
		sell_type(type)

func sell_type(type: int) -> void:
	var amount_sold: float = 0.0
	for pop: base_pop in get_pops():
		var price: float = market.get_local_price(type)
		var amount: float = pop.get_desired(type, price) #Float for each pop
		amount = min(amount, market.get_cargo_amount(type))
		amount_sold += amount
		pop.buy_good(amount, price)
		market.add_cash(amount * price)
	market.report_attempt_to_sell(type, round(amount_sold))
	market.remove_cargo(type, round(amount_sold))

# === Trading to brokers ===

func sell_to_other_brokers() -> void:
	var supply: Array[int] = market.get_supply()
	for type: int in supply:
		var order: trade_order = trade_order.new(type, market.get_cargo_amount(type), false, market.get_local_price(type))
		distribute_from_order(order)

func distribute_from_order(order: trade_order) -> void:
	#Distribute to local factories
	for fact_array: Array in internal_factories.values():
		for fact: factory_template in fact_array:
			if fact.does_accept(order.get_type()):
				distribute_to_order(fact, order)
	#Distribute to stations, ports, or other brokers
	for coords: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_instance().get_broker(coords)
		if broker_obj.does_accept(order.get_type()):
			distribute_to_order(broker_obj, order)

#If amount is positive then ignore since it is taken care of in buy func
func report_attempt(type: int, amount: int) -> void:
	if amount < 0:
		#Only called from selling, so amount is negitive
		market.report_attempt_to_sell(type, -amount)

# === Processes ===

func day_tick() -> void:
	sell_to_other_brokers()

func month_tick() -> void:
	sell_to_pops()
	market.month_tick()
