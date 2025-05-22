class_name army extends Node

static var armies_created: int = 0

const speed_mult_hilly: float = 0.65
const speed_mult_other_unit: float = 0.45

var player_id: int
var army_id: int
var location: Vector2i
var route: Array[Vector2i] = []
#The progress army has to travel
var progress: float

var units: sorted_stack = sorted_stack.new()



func _init(p_player_id: int) -> void:
	player_id = p_player_id
	army_id = armies_created
	armies_created += 1

# == ID Info ==
func get_player_id() -> int:
	return player_id

func get_army_id() -> int:
	return army_id

# == Route Info ==
func get_location() -> Vector2i:
	return location

func set_route(new_route: Array[Vector2i]) -> void:
	route = new_route

func get_next_location(index: int = 0) -> Vector2i:
	if route.is_empty() or index >= route.size():
		return location
	return route[index]

func pop_next_location() -> Vector2i:
	return route.pop_front()

func is_route_empty() -> bool:
	return route.is_empty()

func has_destination() -> bool:
	return !route.is_empty()

func get_destination() -> Variant:
	if route.is_empty():
		return null
	return route.back()

func stop() -> void:
	route = []

func update_progress(num: float) -> void:
	progress += num

func reset_progress() -> void:
	progress = 0

func ready_to_move(progress_needed: float) -> bool:
	if progress_needed < progress:
		reset_progress()
		return true
	return false

static func get_speed_mult(terrain_num: int) -> float:
	if terrain_num == 0:
		return speed_mult_hilly
	else:
		return 1.0

# == Army unit Stuff ==

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

# == Map functions == 
func get_atlas_coord() -> Vector2i:
	#TODO: Add modeling or smth
	return Vector2i(0, 0)

#Returns duplicate
func get_units() -> Array[base_unit]:
	var toReturn: Array[base_unit] = []
	toReturn.assign(units.get_backing_array())
	return toReturn

func merge(army_to_merge_with: army) -> void:
	if army_to_merge_with.get_player_id() != player_id:
		return
	for unit: base_unit in army_to_merge_with.get_units():
		add_unit(unit)
	army_to_merge_with.clear_units()

func clear_units() -> void:
	units.clear()

func split() -> army:
	var new_army: army = army.new(player_id)
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

func convert_to_client_array() -> Array:
	var toReturn: Array[Array] = []
	for unit: base_unit in units:
		toReturn.append(unit.convert_to_client_array())
	return toReturn

func _to_string() -> String:
	var toReturn: String = "["
	for unit: base_unit in get_units():
		toReturn += str(unit) + ", "
	toReturn = toReturn.trim_suffix(", ")
	toReturn += "]"
	return toReturn
