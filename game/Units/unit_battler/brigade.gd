class_name Brigade extends Node

var unit_icon_scene: PackedScene = preload("res://Units/unit_battler/unit_icon.tscn")

var terrain_map: TileMapLayer

var units: Array[unit_icon] = []
var cw: int
var moving_up: bool

func _init(p_cw: int, p_moving_up: bool) -> void:
	cw = p_cw
	moving_up = p_moving_up

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
	var tile: Variant = terrain_map.get_next_open_cell(moving_up)
	if tile == null:
		printerr("Deployed a unit with no where to go")
		return
	unit_icon_obj.set_up(terrain_map.map_to_local(tile))

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

func is_full() -> bool:
	return get_size_of_units() > cw

func day_tick() -> void:
	for unit: unit_icon in units:
		if unit.can_unit_fight():
			unit.day_tick()
		else:
			remove_unit(unit)
