class_name trade_order extends Node

var type: int
var amount: int
#From the perspective of the owner
var buy: bool

func _init(p_type: int, p_amount: int, p_buy: bool) -> void:
	type = p_type
	amount = p_amount
	buy = p_buy

func create_buy_order(p_type: int, p_amount: int) -> void:
	_init(p_type, p_amount, true)

func create_sell_order(p_type: int, p_amount: int) -> void:
	_init(p_type, p_amount, false)

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

func convert_to_array() -> Array:
	return [type, amount, buy]

static func construct_from_array(array: Array) -> void:
	return trade_order.new(array[0], array[1], array[2])
