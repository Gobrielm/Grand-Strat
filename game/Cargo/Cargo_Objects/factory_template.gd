class_name factory_template extends fixed_hold

var DAY_TICKS_PER_MONTH = 15

var trade_orders: Dictionary = {}

var inputs: Dictionary
var outputs: Dictionary

var max_batch_size: int
var local_pricer: local_price_controller

const DEFAULT_BATCH_SIZE = 1

func _init(new_location: Vector2i, _player_owner: int, new_inputs: Dictionary, new_outputs: Dictionary):
	super._init(new_location, _player_owner)
	inputs = new_inputs
	outputs = new_outputs
	max_batch_size = DEFAULT_BATCH_SIZE
	for type in inputs:
		if inputs[type] != 0:
			add_accept(type)
	local_pricer = local_price_controller.new(inputs, outputs)

func add_order(_coords: Vector2i, order: trade_order):
	if !trade_orders.has(_coords):
		trade_orders[_coords] = {}
	trade_orders[_coords][order.get_type()] = order

func edit_order(_coords: Vector2i, order: trade_order):
	trade_orders[_coords][order.get_type()] = order

func remove_order(_coords: Vector2i, _type: int):
	trade_orders[_coords].erase(_type)

func does_create(type: int) -> bool:
	return outputs.has(type)

func get_buy_order_total(type: int) -> int:
	var total = 0
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
	var price = calculate_reward(type, amount)
	remove_cash(price)
	return price

func calculate_reward(type: int, amount: int) -> int:
	return floor(get_local_price(type) * float(amount))

func get_desired_cargo_to_load(type: int) -> int:
	var price = get_local_price(type)
	assert(price > 0)
	var amount: int = get_amount_can_buy(get_local_price(type))
	return min(amount, max_amount - get_cargo_amount(type))

func transfer_cargo(type: int, amount: int) -> int:
	var new_amount = min(storage[type], amount)
	remove_cargo(type, new_amount)
	return new_amount

func create_recipe():
	var batch_size = get_batch_size()
	remove_inputs(batch_size)
	add_outputs(batch_size)

func get_batch_size() -> int:
	var batch_size = max_batch_size
	for index in inputs:
		var amount = inputs[index]
		batch_size = min(floor(storage[index] / amount), batch_size)
	for index in outputs:
		var amount = outputs[index]
		batch_size = min(floor((max_amount - storage[index]) / amount), batch_size)
	return batch_size

func remove_inputs(batch_size: int):
	for index in inputs:
		var amount = inputs[index] * batch_size
		storage[index] -= amount

func add_outputs(batch_size: int):
	for index in outputs:
		var amount = outputs[index] * batch_size
		amount = add_cargo_ignore_accepts(index, amount)
		local_pricer.report_change(index, amount)

func distribute_cargo():
	var array = randomize_station_order()
	for coords in array:
		var _station = terminal_map.get_terminal(coords)
		distribute_to_station(_station)

func randomize_station_order() -> Array:
	var choices: Array = []
	var toReturn: Array = []
	for coords in trade_orders:
		choices.append(coords)
	while !choices.is_empty():
		var rand_num = randi_range(0, choices.size() - 1)
		var choice = choices.pop_at(rand_num)
		toReturn.append(choice)
	return toReturn

func distribute_to_station(_station: station):
	var coords = _station.get_location()
	for type in outputs:
		if trade_orders[coords].has(type):
			var order: trade_order = trade_orders[coords][type]
			if order.is_buy_order():
				distribute_to_order(_station, order)

func distribute_to_order(_station: station, order: trade_order):
	var type = order.get_type()
	var price = get_local_price(type)
	var amount = min(_station.get_desired_cargo_to_load(type, price), order.get_amount(), LOAD_TICK_AMOUNT)
	local_pricer.report_attempt(type, min(amount, outputs[type]))
	amount = transfer_cargo(type, amount)
	_station.buy_cargo(type, amount, price)
	add_cash(amount * price)
	

func day_tick():
	print("Default implementation")
	assert(false)

func month_tick():
	print("Default implementation")
	assert(false)
	
