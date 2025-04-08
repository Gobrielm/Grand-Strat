class_name factory_template extends fixed_hold

const DAY_TICKS_PER_MONTH: int = 15
const COST_FOR_UPGRADE: int = 1000

var level: int
var employment: int
var employment_total: int

var trade_orders: Dictionary = {}

var inputs: Dictionary
var outputs: Dictionary

var local_pricer: local_price_controller

const DEFAULT_BATCH_SIZE: int = 1

func _init(new_location: Vector2i, _player_owner: int, new_inputs: Dictionary, new_outputs: Dictionary) -> void:
	super._init(new_location, _player_owner)
	inputs = new_inputs
	outputs = new_outputs
	for type: int in inputs:
		if inputs[type] != 0:
			add_accept(type)
	local_pricer = local_price_controller.new(inputs, outputs)
	level = 1
	employment_total = 1000

func add_order(order: trade_order) -> void:
	var type: int = order.get_type()
	if trade_orders.has(type):
		edit_order(order)
	trade_orders[type] = order

func edit_order(p_order: trade_order) -> void:
	var type: int = p_order.get_type()
	if trade_orders.has(type):
		trade_orders[type].queue_free()
	trade_orders[type] = p_order

func get_order(type: int) -> trade_order:
	if trade_orders.has(type):
		return trade_orders[type]
	return null

func remove_order(p_type: int) -> void:
	trade_orders.erase(p_type)

func does_create(type: int) -> bool:
	return outputs.has(type)

func get_buy_order_total(type: int) -> int:
	var total: int = 0
	for order_dict: Dictionary in trade_orders.values():
		for order: trade_order in order_dict.values():
			if order.get_type() == type and order.is_buy_order():
				total += min(order.get_amount(), outputs[type])
	return total * DAY_TICKS_PER_MONTH

func get_monthly_demand(type: int) -> int:
	return inputs[type] * DAY_TICKS_PER_MONTH

func get_local_prices() -> Dictionary:
	return local_pricer.local_prices

func get_local_price(type: int) -> float:
	return local_pricer.get_local_price(type)

func buy_cargo(type: int, amount: int) -> int:
	add_cargo(type, amount)
	local_pricer.report_change(type, amount)
	var price: int = calculate_reward(type, amount)
	remove_cash(price)
	return price

func calculate_reward(type: int, amount: int) -> int:
	return floor(get_local_price(type) * float(amount))

func get_desired_cargo_to_load(type: int) -> int:
	var price: float = get_local_price(type)
	assert(price > 0)
	var amount: int = get_amount_can_buy(get_local_price(type))
	return min(amount, max_amount - get_cargo_amount(type))

func transfer_cargo(type: int, amount: int) -> int:
	var new_amount: int = min(storage[type], amount)
	remove_cargo(type, new_amount)
	return new_amount

func create_recipe() -> void:
	var batch_size: int = get_batch_size()
	remove_inputs(batch_size)
	add_outputs(batch_size)

func get_batch_size() -> int:
	var batch_size: int = get_level()
	for index: int in inputs:
		var amount: int = inputs[index]
		batch_size = min(floor(storage[index] / amount), batch_size)
	for index: int in outputs:
		var amount: int = outputs[index]
		batch_size = min(floor((max_amount - storage[index]) / amount), batch_size)
	return batch_size

func remove_inputs(batch_size: int) -> void:
	for index: int in inputs:
		var amount: int = inputs[index] * batch_size
		storage[index] -= amount

func add_outputs(batch_size: int) -> void:
	for index: int in outputs:
		var amount: int = outputs[index] * batch_size
		amount = add_cargo_ignore_accepts(index, amount)
		local_pricer.report_change(index, amount)

func distribute_cargo() -> void:
	var array: Array = randomize_station_order()
	for coords: Vector2i in array:
		#TODO: Is get_terminal, check for better function
		var _station: station = terminal_map.get_terminal(coords)
		distribute_to_station(_station)

func randomize_station_order() -> Array:
	var choices: Array = []
	var toReturn: Array = []
	for coords: Vector2i in trade_orders:
		choices.append(coords)
	while !choices.is_empty():
		var rand_num: int = randi_range(0, choices.size() - 1)
		var choice: int = choices.pop_at(rand_num)
		toReturn.append(choice)
	return toReturn

func distribute_to_station(_station: station) -> void:
	var coords: Vector2i = _station.get_location()
	for type: int in outputs:
		if trade_orders[coords].has(type):
			var order: trade_order = trade_orders[coords][type]
			if order.is_buy_order():
				distribute_to_order(_station, order)

func distribute_to_order(_station: station, order: trade_order) -> void:
	var type: int = order.get_type()
	var price: float = get_local_price(type)
	var amount: int = min(_station.get_desired_cargo_to_load(type, price), order.get_amount(), LOAD_TICK_AMOUNT)
	local_pricer.report_attempt(type, min(amount, outputs[type]))
	amount = transfer_cargo(type, amount)
	_station.buy_cargo(type, amount, price)
	add_cash(round(amount * price))

func get_level() -> int:
	if employment == 0:
		return 0
	return round(float(level * 10) * employment_total / employment)

func get_cost_for_upgrade() -> int:
	return COST_FOR_UPGRADE

func upgrade() -> bool:
	if inputs.is_empty() and outputs.size() == 1:
		var cargo_values: Node = Utils.cargo_values
		var mag: int = cargo_values.get_tile_magnitude(location, outputs[0])
		if mag <= level:
			return false
	level += 1
	employment_total = level * 1000
	return true

func day_tick() -> void:
	print("Default implementation")
	assert(false)

func month_tick() -> void:
	print("Default implementation")
	assert(false)
	
