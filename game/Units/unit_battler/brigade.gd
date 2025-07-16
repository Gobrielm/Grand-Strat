class_name Brigade extends Node

var unit_icon_scene: PackedScene = preload("res://Units/unit_battler/unit_icon.tscn")

var terrain_map: TileMapLayer

var units: Array[unit_icon] = []
var cw: int
var moving_up: bool

var statistics: Dictionary[String, int] = {}
var player_ids: Dictionary[int, bool] = {}

func _init(p_cw: int, p_moving_up: bool) -> void:
	cw = p_cw
	moving_up = p_moving_up
	statistics.inf_dead = 0
	statistics.cav_dead = 0
	statistics.art_dead = 0
	statistics.inf_killed = 0
	statistics.cav_killed = 0
	statistics.art_killed = 0

func assign_terrain_map(p_terrain_map: TileMapLayer) -> void:
	terrain_map = p_terrain_map 

func deploy_unit(unit: base_unit, id: int) -> void:
	player_ids[id] = true
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

func order_attack() -> void:
	for unit: unit_icon in units:
		unit.set_attack()

func order_defend() -> void:
	for unit: unit_icon in units:
		unit.set_defend()

func order_retreat() -> void:
	for unit: unit_icon in units:
		unit.set_retreat()

func add_unit(unit_to_add: unit_icon) -> void:
	units.append(unit_to_add)

func remove_unit(unit: unit_icon) -> void:
	count_stats(unit)
	units.remove_at(units.find(unit))
	remove_child(unit)
	unit.queue_free()

func count_stats(unit: unit_icon) -> void:
	var stats: Dictionary[String, int] = unit.statistics
	if unit.internal_unit is infantry:
		statistics.inf_dead += stats.casualties
	elif unit.internal_unit is calvary:
		statistics.cav_dead += stats.casualties
	else:
		statistics.art_dead += stats.casualties
	
	statistics.inf_killed += stats.inf_killed
	statistics.cav_killed += stats.cav_killed
	statistics.art_killed += stats.art_killed

func count_remaining_units() -> void:
	for unit: unit_icon in units:
		count_stats(unit)

func get_units() -> Array[unit_icon]:
	return units

func get_size_of_units() -> int:
	return units.size()

func is_full() -> bool:
	@warning_ignore("integer_division")
	return get_size_of_units() >= (cw / 2)

func is_empty() -> bool:
	return get_size_of_units() == 0

func get_unit_center_of_mass() -> Vector2:
	var com: Vector2 = Vector2(0, 0)
	for unit: unit_icon in units:
		com += unit.position
	com /= units.size()
	return com

func get_player_ids() -> Dictionary:
	return player_ids

func day_tick() -> void:
	for unit: unit_icon in units:
		if unit.can_unit_fight():
			unit.day_tick()
		else:
			remove_unit(unit)

func process(delta: float) -> void:
	for unit: unit_icon in units:
		if unit.can_unit_fight():
			unit.process(delta)
		else:
			remove_unit(unit)
