class_name trade_order extends Node

var type: int
var amount: int
var buy: bool
var coords_of_factory: Vector2i

func _init(_type: int, _amount: int, _buy: bool, _coords_of_factory: Vector2i):
	type = _type
	amount = _amount
	buy = _buy
	coords_of_factory = _coords_of_factory

func create_buy_order(_type: int, _amount: int, _coords_of_factory: Vector2i):
	_init(_type, _amount, true, _coords_of_factory)

func create_sell_order(_type: int, _amount: int, _coords_of_factory: Vector2i):
	_init(_type, _amount, false, _coords_of_factory)

func is_buy_order() -> bool:
	return buy

func is_sell_order() -> bool:
	return !buy

func change_buy(_buy: bool):
	buy = _buy

func get_type() -> int:
	return type

func change_amount(_amount: int):
	amount = _amount

func get_amount() -> int:
	return amount

func get_coords_of_factory() -> Vector2i:
	return coords_of_factory

func convert_to_array() -> Array:
	return [type, amount, buy, coords_of_factory]

static func construct_from_array(array: Array):
	return trade_order.new(array[0], array[1], array[2], array[3])
