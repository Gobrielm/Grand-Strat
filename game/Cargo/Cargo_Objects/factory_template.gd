class_name factory_template extends broker

const DAY_TICKS_PER_MONTH: int = 15
const COST_FOR_UPGRADE: int = 1000

var level: int
var employment: int
var employment_total: int

var inputs: Dictionary
var outputs: Dictionary

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

func buy_cargo(type: int, amount: int, price_per: float) -> void:
	if amount == 0:
		return
	add_cargo_ignore_accepts(type, amount)
	remove_cash(round(amount * price_per))
	local_pricer.report_change(type, amount)

func calculate_reward(type: int, amount: int) -> int:
	return floor(get_local_price(type) * float(amount))

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
	for type: int in outputs:
		if trade_orders.has(type):
			var order: trade_order = trade_orders[type]
			if order.is_sell_order():
				distribute_from_order(order)

func distribute_from_order(order: trade_order) -> void:
	var done: bool = false
	var dir_term_options: Array[broker] = get_directly_connected_terms(order.get_type())
	var term_options: Array[broker] = get_connected_terms(order.get_type())
	
	while !done and !term_options.is_empty() and !dir_term_options.is_empty():
		var broker_obj: broker = get_broker_obj(term_options, dir_term_options, order.get_type())
		done = distribute_to_order(broker_obj, order)
	amount_traded = 0

func get_broker_obj(term_options: Array[broker], dir_term_options: Array[broker], type: int) -> broker:
	if dir_term_options.is_empty() or (!term_options.is_empty() and term_options.front().get_local_price(type) > dir_term_options.front().get_local_price(type)):
		return term_options.pop_front()
	else:
		return dir_term_options.pop_front()

func distribute_to_order(_broker: broker, order: trade_order) -> bool:
	var type: int = order.get_type()
	#TODO: price chooses highest between seller and buyer, consider changing to matching the buyer more
	var price: float = max(get_local_price(type), _broker.get_local_price(type))
	
	var amount: int = min(_broker.get_desired_cargo_to_load(type, price), order.get_amount(), LOAD_TICK_AMOUNT - amount_traded)
	if amount > 0:
		local_pricer.report_attempt(type, min(amount, outputs[type]))
		amount = transfer_cargo(type, amount)
		amount_traded += amount
		_broker.buy_cargo(type, amount, price)
		add_cash(round(amount * price))
	
	if amount_traded >= LOAD_TICK_AMOUNT:
		return true
	return false 

func get_level() -> int:
	#TODO: Employment
	#if employment == 0:
		#return 0
	#return round(float(level * 10) * employment_total / employment)
	return level

func get_cost_for_upgrade() -> int:
	return COST_FOR_UPGRADE

func upgrade() -> bool:
	if inputs.is_empty() and outputs.size() == 1:
		var cargo_values: Node = Utils.cargo_values
		var mag: int = cargo_values.get_tile_magnitude(location, outputs.values()[0])
		if mag > level:
			var cost: int = get_cost_for_upgrade()
			if cash >= cost:
				remove_cash(cost)
				level += 1
				employment_total = level * 1000
				return true
	return false

func day_tick() -> void:
	print("Default implementation")
	assert(false)

func month_tick() -> void:
	print("Default implementation")
	assert(false)
	
