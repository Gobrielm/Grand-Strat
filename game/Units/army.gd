class_name army extends Node

var units: sorted_stack = sorted_stack.new()

var location: Vector2i

func _init() -> void:
	pass

func add_unit(unit: base_unit) -> void:
	units.insert_element(unit, get_unit_weight(unit))

func remove_unit(index: int) -> void:
	units.remove_element(index)

func get_unit_weight(unit: base_unit) -> float:
	if unit is infantry:
		return 4
	elif unit is calvary:
		return 3
	elif unit is artillery:
		return 2
	elif unit is engineer:
		return 1
	elif unit is officer:
		return 0
	return -1

#Returns duplicate
func get_units() -> Array[base_unit]:
	var toReturn: Array[base_unit] = []
	toReturn.assign(units.get_backing_array())
	return toReturn

func merge(army_to_merge_with: army) -> void:
	for unit: base_unit in army_to_merge_with.get_units():
		units.append(unit)
	army_to_merge_with.clear_units()

func clear_units() -> void:
	units.clear()

func split() -> army:
	var new_army: army = army.new()
	var counter: bool = false
	var units_array: Array[base_unit] = get_units()
	var units_removed: int = 0
	for index: int in units_array.size():
		var unit: base_unit = units_array[index]
		if counter:
			new_army.add_unit(unit)
			remove_unit(index - units_removed)
			units_removed += 1
		counter = !counter
	
	return new_army

func _to_string() -> String:
	var toReturn: String = "["
	for unit: base_unit in get_units():
		toReturn += str(unit) + ", "
	toReturn = toReturn.trim_suffix(", ")
	toReturn += "]"
	return toReturn
