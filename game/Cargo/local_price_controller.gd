class_name local_price_controller extends Node

const MAX_DIFF: float = 1.5

var attempts_to_buy: Dictionary = {}
var change: Dictionary = {}
var local_prices: Dictionary = {}
static var base_prices: Dictionary

func _init(inputs: Dictionary, outputs: Dictionary) -> void:
	for type: int in inputs:
		local_prices[type] = base_prices[type]
		reset_change(type)
	for type: int in outputs:
		local_prices[type] = base_prices[type]
		reset_change(type)
		reset_attempts(type)

static func set_base_prices(p_base_prices: Dictionary) -> void:
	base_prices = p_base_prices

func report_change(type: int, amount: int) -> void:
	change[type] += amount

func report_attempt(type: int, amount: int) -> void:
	attempts_to_buy[type] += amount

func reset_change(type: int) -> void:
	change[type] = 0

func reset_attempts(type: int) -> void:
	attempts_to_buy[type] = 0

func get_change(type: int) -> int:
	return change[type]

func get_attempts(type: int) -> int:
	return attempts_to_buy[type]

func get_local_price(type: int) -> int:
	return local_prices[type]

func get_percent_difference(type: int) -> float:
	return 2 * (local_prices[type] - base_prices[type]) / (local_prices[type] + base_prices[type])

func get_base_price(type: int) -> int:
	return base_prices[type]

func vary_input_price(demand: int, type: int) -> void:
	vary_buy_order(demand, get_change(type), type)
	reset_change(type)

func vary_output_price(type: int) -> void:
	vary_sell_order(get_attempts(type), get_change(type), type)
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
	if supply == 0:
		#TODO: THIS MAY CAUSE FUTURE PROBLEMS BY IGNORING
		return
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
	var max_multiple: float = 1 / percentage_met
	var multiple: float = get_multiple(type)
	if multiple >= max_multiple:
		multiple = max_multiple
	elif multiple >= MAX_DIFF:
		multiple = MAX_DIFF
	else:
		multiple += (0.01 * amount)
	local_prices[type] = base_prices[type] * multiple

func bump_down_good_price(type: int, percentage_met: float, amount: int) -> void:
	var multiple: float = get_multiple(type)
	if multiple <= percentage_met:
		multiple = percentage_met
	elif multiple <= 1 / MAX_DIFF:
		multiple = 1 / MAX_DIFF
	else:
		multiple -= (0.01 * amount)
	local_prices[type] = base_prices[type] * multiple

func equalize_good_price(type: int) -> void:
	var multiple: float = get_multiple(type)
	if multiple > 1:
		bump_down_good_price(type, 1, 1)
	elif multiple < 1:
		bump_up_good_price(type, 1, 1)
