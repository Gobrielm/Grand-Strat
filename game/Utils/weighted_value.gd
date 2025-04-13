class_name weighted_value extends Node

var val

var weight: float

func _init(p_val, p_weight: float) -> void:
	val = p_val
	weight = p_weight

func _to_string() -> String:
	return "Weight: " + str(weight)
	 #+ ", Value: " + str(val)
