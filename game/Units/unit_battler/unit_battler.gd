extends Control

# Defender is always on the top

@onready var terrain_map: TileMapLayer = $unit_battler_terrain_map
var cw: int = 0
var attacking_armies: Array[army] = []
var defending_armies: Array[army] = []

var deployed_def_units: Array[base_unit] = []
var deployed_atk_units: Array[base_unit] = []

var atk_frontline: FrontLine
var def_frontline: FrontLine

func _ready() -> void:
	pass

func add_def_army(army_obj: army) -> void:
	defending_armies.append(army_obj)

func add_atk_army(army_obj: army) -> void:
	attacking_armies.append(army_obj)

func set_cw(p_cw: int) -> void:
	cw = p_cw
	terrain_map.set_cw(p_cw)

func start_battle() -> void:
	atk_frontline = FrontLine.create_with_nums(15, cw)
	def_frontline = FrontLine.create_with_nums(-15, cw)
	add_child(atk_frontline)
	add_child(def_frontline)
	deploy_units(atk_frontline, attacking_armies)
	deploy_units(def_frontline, defending_armies)
	atk_frontline.set_up_units()
	def_frontline.set_up_units()
	atk_frontline.pt1 = Vector2(-cw * 64, 0)
	atk_frontline.pt2 = Vector2(cw * 64, 0)
	def_frontline.pt1 = Vector2(-cw * 64, 0)
	def_frontline.pt2 = Vector2(cw * 64, 0)
	atk_frontline.assign_dest_for_units()
	def_frontline.assign_dest_for_units()

func deploy_units(frontline: FrontLine, armies: Array[army]) -> void:
	for army_obj: army in armies:
		var units: Array[base_unit] = army_obj.get_units().duplicate()
		units.shuffle()
		for unit: base_unit in units:
			if unit.can_unit_fight():
				frontline.deploy_unit(unit, army_obj.get_player_id())
				if frontline.is_line_full():
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
	atk_frontline.day_tick()
	def_frontline.day_tick()
	
	#if atk_frontline.can_move():
		#atk_frontline.move_line(Vector2(0, -128))
	#if def_frontline.can_move():
		#def_frontline.move_line(Vector2(0, 128))
