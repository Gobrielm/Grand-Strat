class_name request extends RefCounted

var type: int
var amount: int
var source: Vector2i
var max_price: float

func _init(p_type: int, p_amount: int, p_source: Vector2i, p_max_price: float) -> void:
	type = p_type
	amount = p_amount
	source = p_source
	max_price = p_max_price
