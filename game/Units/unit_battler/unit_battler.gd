extends Control

# Defender is always on the top

@onready var terrain_map: TileMapLayer = $unit_battler_terrain_map
var cw: int = 0
var attacking_armies: Array[army] = []
var defending_armies: Array[army] = []

var deployed_def_units: Array[base_unit] = []
var deployed_atk_units: Array[base_unit] = []

var atk_brigade: Brigade
var def_brigade: Brigade

var ticks: float = 0
const SECONDS_IN_DAY: int = 1

func _process(delta: float) -> void:
	delta *= 4
	ticks += delta
	if atk_brigade != null:
		atk_brigade.process(delta)
	if def_brigade != null:
		def_brigade.process(delta)
	if ticks >= SECONDS_IN_DAY:
		day_tick()
		ticks = 0

func _ready() -> void:
	pass

func add_def_army(army_obj: army) -> void:
	defending_armies.append(army_obj)

func add_atk_army(army_obj: army) -> void:
	attacking_armies.append(army_obj)

func set_cw(p_cw: int) -> void:
	cw = p_cw
	terrain_map.set_cw(p_cw)

func get_other_side_com(moving_up: bool) -> Vector2:
	return def_brigade.get_unit_center_of_mass() if moving_up else atk_brigade.get_unit_center_of_mass()

func get_other_side_brigade(moving_up: bool) -> Brigade:
	return def_brigade if moving_up else atk_brigade

func start_battle() -> void:
	atk_brigade = Brigade.new(cw, true)
	def_brigade = Brigade.new(cw, false)
	atk_brigade.assign_terrain_map(terrain_map)
	def_brigade.assign_terrain_map(terrain_map)
	add_child(atk_brigade)
	add_child(def_brigade)
	deploy_units(atk_brigade, attacking_armies)
	deploy_units(def_brigade, defending_armies)
	atk_brigade.order_attack()

func deploy_units(brigade: Brigade, armies: Array[army]) -> void:
	for army_obj: army in armies:
		var units: Array[base_unit] = army_obj.get_units().duplicate()
		units.shuffle()
		for unit: base_unit in units:
			if unit.can_unit_fight():
				brigade.deploy_unit(unit, army_obj.get_player_id())
				if brigade.is_full():
					return

func remove_def_unit(unit: unit_icon) -> void:
	remove_child(unit)
	unit.queue_free()
	var i: int = deployed_def_units.find(unit.internal_unit)
	deployed_def_units.remove_at(i)

func remove_atk_unit(unit: unit_icon) -> void:
	remove_child(unit)
	unit.queue_free()
	var i: int = deployed_atk_units.find(unit.internal_unit)
	deployed_atk_units.remove_at(i)

func day_tick() -> void:
	atk_brigade.day_tick()
	def_brigade.day_tick()
