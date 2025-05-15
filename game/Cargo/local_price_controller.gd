class_name local_price_controller extends Node

const MAX_DIFF: float = 1.5

#TODO: Redo to have buy/sell attempts

var change: Dictionary[int, int] = {}
var attempts_to_trade: Dictionary = {}
var local_prices: Dictionary = {}
static var base_prices: Dictionary

func _init(inputs: Dictionary, outputs: Dictionary) -> void:
	for type: int in inputs:
		add_cargo_type(type)
	for type: int in outputs:
		add_cargo_type(type)

static func set_base_prices(p_base_prices: Dictionary) -> void:
	base_prices = p_base_prices

func add_cargo_type(type: int, starting_price: float = base_prices[type]) -> void:
	local_prices[type] = starting_price
	reset_attempts(type)
	reset_change(type)

func remove_cargo_type(type: int) -> void:
	local_prices.erase(type)
	attempts_to_trade.erase(type)
	change.erase(type)

func add_cargo_from_factory(fact: factory_template) -> void:
	for type: int in fact.outputs:
		add_cargo_type(type)

func get_change(type: int) -> int:
	return change[type]

func reset_change(type: int) -> void:
	change[type] = 0

func report_change(type: int, amount: int) -> void:
	change[type] += amount

func report_attempt(type: int, amount: int) -> void:
	attempts_to_trade[type] += amount

func reset_attempts(type: int) -> void:
	attempts_to_trade[type] = 0

func get_attempts(type: int) -> int:
	return attempts_to_trade[type]

func get_local_price(type: int) -> int:
	return local_prices[type]

func get_percent_difference(type: int) -> float:
	return 2 * (local_prices[type] - base_prices[type]) / (local_prices[type] + base_prices[type])

func get_base_price(type: int) -> int:
	return base_prices[type]

func vary_input_price(demand: int, type: int) -> void:
	if get_change(type) == 0:
		vary_buy_order(demand, 0, type)
	else:
		vary_buy_order(demand, get_attempts(type), type)
	reset_attempts(type)
	reset_change(type)

func vary_output_price(supply: int, type: int) -> void:
	if get_change(type) == 0:
		vary_sell_order(0, supply, type)
	else:
		vary_sell_order(get_attempts(type), supply, type)
	reset_attempts(type)
	reset_change(type)

func vary_buy_order(demand: int, supply: int, type: int) -> void:
	assert(demand != 0)
	var percentage_being_met: float = 1 - float(demand - supply) / demand
	if demand / 1.1 > supply:
		bump_up_good_price(type, percentage_being_met, 1)
	elif demand * 1.1 < supply:
		bump_down_good_price(type, percentage_being_met, 2)
	else:
		equalize_good_price(type)

func vary_sell_order(demand: int, supply: int, type: int) -> void:
	assert(supply != 0)
	var percentage_being_met: float = 1 - float(supply - demand) / supply
	if demand / 1.1 > supply:
		bump_up_good_price(type, percentage_being_met, 2)
	elif demand * 1.1 < supply:
		bump_down_good_price(type, percentage_being_met, 1)
	else:
		equalize_good_price(type)

func get_multiple(type: int) -> float:
	return local_prices[type] /  base_prices[type]

func bump_up_good_price(type: int, percentage_met: float, amount: int) -> void:
	#Set to 0 by default
	var max_multiple: float = 0.0
	var price_disparity: float = 0.5
	
	var multiple: float = get_multiple(type)
	
	if percentage_met != 0:
		max_multiple = 1 / percentage_met
		price_disparity = abs(multiple - percentage_met)
	
	if percentage_met != 0 and multiple >= max_multiple:
		multiple = max_multiple
	elif multiple >= MAX_DIFF:
		multiple = MAX_DIFF
	else:
		multiple += (max(price_disparity / 10, 0.01) * amount)
	
	local_prices[type] = base_prices[type] * multiple

func bump_down_good_price(type: int, percentage_met: float, amount: int) -> void:
	#Can't have 0 percentage met
	var multiple: float = get_multiple(type)
	var price_disparity: float = abs(multiple - percentage_met)
	
	
	if multiple <= percentage_met:
		multiple = percentage_met
	elif multiple <= 1 / MAX_DIFF:
		multiple = 1 / MAX_DIFF
	else:
		multiple -= (max(price_disparity / 10, 0.01) * amount)
	local_prices[type] = base_prices[type] * multiple

func equalize_good_price(type: int) -> void:
	var multiple: float = get_multiple(type)
	if multiple > 1:
		bump_down_good_price(type, 1, 1)
	elif multiple < 1:
		bump_up_good_price(type, 1, 1)
