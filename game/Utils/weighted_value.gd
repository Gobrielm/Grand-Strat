class_name weighted_value extends RefCounted

var val: Variant

var weight: float

func _init(p_val: Variant, p_weight: float) -> void:
	val = p_val
	weight = p_weight

func _to_string() -> String:
	return "Weight: " + str(weight)
	 #+ ", Value: " + str(val)
