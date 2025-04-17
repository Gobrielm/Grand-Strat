class_name trade_order extends Node

var type: int
var amount: int
#From the perspective of the owner
var buy: bool
var max_price: float

func _init(p_type: int, p_amount: int, p_buy: bool, p_max_price: float) -> void:
	type = p_type
	amount = p_amount
	buy = p_buy
	max_price = p_max_price

func create_buy_order(p_type: int, p_amount: int, p_max_price: float) -> void:
	_init(p_type, p_amount, true, p_max_price)

func create_sell_order(p_type: int, p_amount: int, p_max_price: float) -> void:
	_init(p_type, p_amount, false, p_max_price)

func is_buy_order() -> bool:
	return buy

func is_sell_order() -> bool:
	return !buy

func change_buy(_buy: bool) -> void:
	buy = _buy

func get_type() -> int:
	return type

func change_amount(p_amount: int) -> void:
	amount = p_amount

func get_amount() -> int:
	return amount

func get_max_price() -> float:
	return max_price

func set_max_price(p_max_price: float) -> void:
	max_price = p_max_price

func convert_to_array() -> Array:
	return [type, amount, buy, max_price]

static func construct_from_array(array: Array) -> trade_order:
	return trade_order.new(array[0], array[1], array[2], array[3])
