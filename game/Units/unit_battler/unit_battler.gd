extends Control

# Defender is always on the top

@onready var terrain_map: TileMapLayer = $unit_battler_terrain_map
var cw: int = 0
var attacking_armies: Array[army] = []
var defending_armies: Array[army] = []

var deployed_def_units: Array[base_unit] = []
var deployed_atk_units: Array[base_unit] = []

var unit_icon_scene: PackedScene = preload("res://Units/unit_battler/unit_icon.tscn")

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
	deploy_atk_units()
	deploy_def_units()
	atk_frontline.set_up_units()
	def_frontline.set_up_units()

func deploy_atk_units() -> void:
	for army_obj: army in attacking_armies:
		for unit: base_unit in army_obj.get_units():
			if unit.can_unit_fight():
				deploy_atk_unit(unit, army_obj.get_player_id())
				if deployed_atk_units.size() == cw * 2:
					return

func deploy_def_units() -> void:
	for army_obj: army in defending_armies:
		for unit: base_unit in army_obj.get_units():
			if unit.can_unit_fight():
				deploy_def_unit(unit, army_obj.get_player_id())
				if deployed_def_units.size() == cw * 2:
					return

func deploy_atk_unit(unit: base_unit, id: int) -> void:
	deployed_atk_units.append(unit)
	var unit_icon_obj: unit_icon = unit_icon_scene.instantiate()
	unit_icon_obj.assign_unit(unit)
	unit_icon_obj.assign_id(id)
	add_child(unit_icon_obj)
	atk_frontline.add_unit(unit_icon_obj)

func deploy_def_unit(unit: base_unit, id: int) -> void:
	deployed_def_units.append(unit)
	var unit_icon_obj: unit_icon = unit_icon_scene.instantiate()
	unit_icon_obj.assign_unit(unit)
	unit_icon_obj.assign_id(id)
	add_child(unit_icon_obj)
	def_frontline.add_unit(unit_icon_obj)

func day_tick() -> void:
	atk_frontline.day_tick()
	def_frontline.day_tick()
	
	if atk_frontline.can_move():
		atk_frontline.move_line(Vector2(0, -128))
	if def_frontline.can_move():
		def_frontline.move_line(Vector2(0, 128))
