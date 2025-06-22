extends RefCounted

var map: TileMapLayer
var combat_width: int

enum combat_results {
	DEF_LOSS = 1,
	ATK_LOSS = 2,
	STALEMATE = -1,
}

func _init(new_map: TileMapLayer) -> void:
	map = new_map
# Neds to eventually have centered units and flanking
func battle_tick(attacking_armies: Array[army], defending_armies: Array[army]) -> combat_results:
	var atk_frontline_units: Array[base_unit] = get_frontline_troops(attacking_armies)
	var def_frontline_units: Array[base_unit] = get_frontline_troops(defending_armies)
	
	var atk_backline_units: Array[base_unit] = get_backline_troops(attacking_armies)
	var def_backline_units: Array[base_unit] = get_backline_troops(defending_armies)
	
	var relevant_cbt_wdth: int = max(atk_frontline_units.size(), def_frontline_units.size())
	for i: int in range(relevant_cbt_wdth):
		# Backline units
		var def_back: base_unit = def_backline_units[i]
		var atk_back: base_unit = atk_backline_units[i]
		# Frontline units
		var def: base_unit = def_frontline_units[i]
		var atk: base_unit = atk_frontline_units[i]
		# Defender backline
		var result: combat_results = unit_battle(atk, def_back)
		if result == combat_results.DEF_LOSS:
			def.set_can_fight(false)
		# Attacker backline
		result = unit_battle(atk_back, def)
		if result == combat_results.ATK_LOSS:
			atk.set_can_fight(false)
		
		# Front on Front
		result = unit_battle(atk, def)
		if result == combat_results.DEF_LOSS:
			def.set_can_fight(false)
		elif result == combat_results.ATK_LOSS:
			atk.set_can_fight(false)
	
	# Defenders win
	if is_battle_over(atk_frontline_units):
		return combat_results.ATK_LOSS
	# Attackers win
	elif is_battle_over(def_frontline_units):
		return combat_results.DEF_LOSS
	
	return combat_results.STALEMATE

func get_frontline_troops(armies: Array[army]) -> Array[base_unit]:
	var units: Array[base_unit] = []
	for army_obj: army in armies:
		for unit: base_unit in army_obj.get_units():
			if unit.get_unit_range() == 1 and unit.can_unit_fight():
				units.append(unit)
				if units.size() == combat_width:
					return units
	return units

func get_backline_troops(armies: Array[army]) -> Array[base_unit]:
	var units: Array[base_unit] = []
	for army_obj: army in armies:
		for unit: base_unit in army_obj.get_units():
			if unit.get_unit_range() > 1 and unit.can_unit_fight():
				units.append(unit)
				if units.size() == combat_width:
					return units
	return units

#Attacker goes first which gives advantage but defender also gets bonuses
func unit_battle(attacker: base_unit, defender: base_unit) -> combat_results:
	var def_fire: int = defender.get_fire_damage()
	var def_shock: int = defender.get_shock_damage()
	var dist: int = check_range_to_unit(defender, attacker)
	var def_range: int = defender.get_unit_range()
	
	attacker.add_battle_experience()
	defender.add_battle_experience()
	defender.remove_manpower(attacker.get_fire_damage())
	defender.remove_morale(attacker.get_shock_damage())
	
	
	if defender.get_manpower() == 0 or defender.get_morale() == 0:
		#kill unit
		return combat_results.DEF_LOSS
	
	if def_range >= dist:
		attacker.remove_manpower(def_fire)
		attacker.remove_morale(def_shock)
	
	if attacker.get_manpower() == 0 or attacker.get_morale() == 0:
		#kill unit
		return combat_results.ATK_LOSS
	
	return combat_results.STALEMATE

func is_battle_over(army_obj: Array[base_unit]) -> bool:
	for unit: base_unit in army_obj:
		if unit.get_can_fight():
			return false
	return true

func conclude_battle() -> void:
	pass

func check_range_to_unit(defender: base_unit, attacker: base_unit) -> int:
	var coords_of_defender: Vector2i = defender.get_location()
	var coords_of_attacker: Vector2i = attacker.get_location()
	for tile: Vector2i in map.get_surrounding_cells(coords_of_defender):
		if tile == coords_of_attacker:
			return 1
	return 2
	
