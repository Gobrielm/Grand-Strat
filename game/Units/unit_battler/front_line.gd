class_name FrontLine extends Object

var units: Array[unit_icon] = []
var pt1: Vector2
var pt2: Vector2

const SIZE_OF_UNIT_PX: int = 10

func _init(new_pt1: Vector2, new_pt2: Vector2) -> void:
	pt1 = new_pt1
	pt2 = new_pt2

static func create_with_nums(back_y: int, cw: int) -> FrontLine:
	@warning_ignore("integer_division")
	var new_pt1: Vector2 = Vector2(-cw * 128 / 2, back_y * 128)
	@warning_ignore("integer_division")
	var new_pt2: Vector2 = Vector2(cw * 128 / 2, back_y * 128)
	return FrontLine.new(new_pt1, new_pt2)

func get_length() -> float:
	return pt1.distance_to(pt2)

func get_max_cw() -> int:
	return floor(get_length() / SIZE_OF_UNIT_PX)

func add_unit(unit_to_add: unit_icon) -> void:
	units.append(unit_to_add)

func remove_unit(index: int) -> unit_icon:
	return units.pop_at(index)

func get_units() -> Array[unit_icon]:
	return units

func get_size_of_units() -> int:
	return units.size()

func assign_dest_for_units() -> void:
	var diff: Vector2 = Vector2(0, 0)
	var move_each: Vector2 = pt2 - pt1
	move_each.x /= (get_size_of_units() - 1)
	move_each.y /= (get_size_of_units() - 1)
	for unit: unit_icon in units:
		unit.set_route(pt1 + diff)
		diff += move_each

func set_up_units() -> void:
	var diff: Vector2 = Vector2(0, 0)
	var move_each: Vector2 = pt2 - pt1
	move_each.x /= (get_size_of_units() - 1)
	move_each.y /= (get_size_of_units() - 1)
	for unit: unit_icon in units:
		unit.position = (pt1 + diff)
		diff += move_each

func move_line(position_to_add: Vector2) -> void:
	pt1 += position_to_add
	pt2 += position_to_add
	assign_dest_for_units()

func day_tick() -> void:
	for unit: unit_icon in units:
		unit.day_tick()

func can_move() -> bool:
	for unit: unit_icon in units:
		if unit.can_unit_attack() or unit.has_route():
			return false
	return true
