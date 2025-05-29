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

# === Pops ===

func get_pops() -> Array[base_pop]:
	var toReturn: Array[base_pop]
	toReturn.assign(city_pops.values())
	return toReturn

func sell_to_pops() -> void:
	for type: int in market.supply:
		sell_type(type)

func sell_type(type: int) -> void:
	var amount_sold: float = 0.0
	for pop: base_pop in get_pops():
		var amount: float = pop.get_desired(type) #Float for each pop
		amount = min(amount, market.get_cargo_amount(type))
		amount_sold += amount
		pop.buy_good(amount, market.get_local_price(type))
		market.add_cash(amount * market.get_local_price(type))
	market.report_attempt_to_sell(type, round(amount_sold))
	market.remove_cargo(type, round(amount_sold))

# === Trading to brokers ===

func sell_to_other_brokers() -> void:
	for type: int in market.supply:
		distribute_from_order(trade_order.new(type, market.get_cargo_amount(type), false, market.get_local_price(type)))

# === Processes ===

func day_tick() -> void:
	sell_to_pops()
	sell_to_other_brokers()

func month_tick() -> void:
	market.month_tick()
