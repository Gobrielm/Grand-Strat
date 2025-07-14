class_name FrontLine extends Node

var unit_icon_scene: PackedScene = preload("res://Units/unit_battler/unit_icon.tscn")

var terrain_map: TileMapLayer

var units: Array[unit_icon] = []

var target: Vector2
var is_target_valid: bool = false

var pt1: Vector2
var pt2: Vector2

const SIZE_OF_UNIT_PX: int = 128

func _init(new_pt1: Vector2, new_pt2: Vector2) -> void:
	pt1 = new_pt1
	pt2 = new_pt2

static func create_with_nums(back_y: int, cw: int) -> FrontLine:
	@warning_ignore("integer_division")
	var new_pt1: Vector2 = Vector2(-cw * 128 / 2, back_y * 128)
	@warning_ignore("integer_division")
	var new_pt2: Vector2 = Vector2(cw * 128 / 2, back_y * 128)
	return FrontLine.new(new_pt1, new_pt2)

func assign_terrain_map(p_terrain_map: TileMapLayer) -> void:
	terrain_map = p_terrain_map 

func deploy_unit(unit: base_unit, id: int) -> void:
	var unit_icon_obj: unit_icon = unit_icon_scene.instantiate()
	unit_icon_obj.assign_unit(unit)
	unit_icon_obj.assign_terrain_map(terrain_map)
	unit_icon_obj.assign_id(id)
	add_child(unit_icon_obj)
	add_unit(unit_icon_obj)
	unit_icon_obj.set_defend()

func add_unit(unit_to_add: unit_icon) -> void:
	units.append(unit_to_add)

func remove_unit(unit: unit_icon) -> void:
	units.remove_at(units.find(unit))
	remove_child(unit)
	unit.queue_free()

func get_units() -> Array[unit_icon]:
	return units

func get_size_of_units() -> int:
	return units.size()

func day_tick() -> void:
	for unit: unit_icon in units:
		if unit.can_unit_fight():
			unit.day_tick()
		else:
			remove_unit(unit)

func set_target(p_target: Vector2) -> void:
	target = p_target
	is_target_valid = true


func is_line_full() -> bool:
	return get_max_cw() == get_size_of_units()

func get_length() -> float:
	return pt1.distance_to(pt2)

func get_max_cw() -> int:
	return floor(get_length() / SIZE_OF_UNIT_PX)

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
		unit.set_up(pt1 + diff)
		diff += move_each

func move_line(position_to_add: Vector2) -> void:
	pt1 += position_to_add
	pt2 += position_to_add
	assign_dest_for_units()

func can_move() -> bool:
	for unit: unit_icon in units:
		if unit.can_unit_attack() or unit.has_route():
			return false
	return true
