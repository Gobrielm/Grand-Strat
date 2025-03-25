extends Node

var map: TileMapLayer

func _init(new_map):
	map = new_map

#Attacker goes first which gives advantage but defender also gets bonuses
func unit_battle(attacker: base_unit, defender: base_unit) -> int:
	if attacker.get_player_id() == defender.get_player_id():
		return -1
	var def_fire = defender.get_fire_damage()
	var def_shock = defender.get_shock_damage()
	var dist = check_range_to_unit(defender, attacker)
	var def_range = defender.get_unit_range()
	
	defender.add_battle_experience()
	defender.remove_manpower(attacker.get_fire_damage())
	defender.remove_morale(attacker.get_shock_damage())
	if defender.get_manpower() == 0:
		#kill unit
		return 0
	elif defender.get_morale() == 0:
		#Retreat unit
		return 1
	if def_range >= dist:
		attacker.add_battle_experience()
		attacker.remove_manpower(def_fire)
		attacker.remove_morale(def_shock)
	
	if attacker.get_manpower() == 0:
		#kill unit
		return 2
	elif attacker.get_morale() == 0:
		#Retreat unit
		return 3
	
	return -1
	

func check_range_to_unit(defender: base_unit, attacker: base_unit) -> int:
	var coords_of_defender: Vector2i = defender.get_location()
	var coords_of_attacker: Vector2i = attacker.get_location()
	for tile in map.get_surrounding_cells(coords_of_defender):
		if tile == coords_of_attacker:
			return 1
	return 2
	
