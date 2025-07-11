class_name Brigade extends Node

var unit_icon_scene: PackedScene = preload("res://Units/unit_battler/unit_icon.tscn")

var units: Array[unit_icon] = []

var target: Vector2
var is_target_valid: bool = false

func deploy_unit(unit: base_unit, id: int) -> void:
	var unit_icon_obj: unit_icon = unit_icon_scene.instantiate()
	unit_icon_obj.assign_unit(unit)
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
