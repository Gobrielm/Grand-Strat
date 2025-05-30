class_name cargo_collection extends RefCounted

var type: int
var origination: Vector2i
var amount: int
var time: Dictionary = {}

func _init(new_type: int, new_origination: Vector2i):
	type = new_type
	origination = new_origination
	amount = 0

func set_time_if_null():
	if time.is_empty():
		time = Time.get_time_dict_from_system()

func is_origination_same(other_origination: Vector2i) -> bool:
	return other_origination == origination

func is_type_same(other_type: int) -> bool:
	return other_type == type

func change_amount(change: int):
	amount += change

func transfer_amount(amount_wanted) -> int:
	var toReturn = min(amount, amount_wanted)
	change_amount(-toReturn)
	return toReturn

func transfer_all() -> int:
	var toReturn = amount
	change_amount(-amount)
	return toReturn

func get_amount() -> int:
	return amount

func get_type() -> int:
	return type

func get_origination() -> Vector2i:
	return origination

func get_time() -> int:
	var current_time = Time.get_time_dict_from_system()
	var diff = compute_difference_of_times(current_time, time)
	return diff

func compute_difference_of_times(time1: Dictionary, time2: Dictionary) -> int:
	if time2.is_empty():
		return 0
	var difference: Dictionary = {}
	difference["hour"] = time1["hour"] - time2["hour"]
	difference["minute"] = time1["minute"] - time2["minute"]
	difference["second"] = time1["second"] - time2["second"]
	return difference["hour"] * 3600 + difference["minute"] * 60 + difference["second"]

func get_distance(other: Vector2i) -> int:
	return round((other - origination).length())
