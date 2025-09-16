class_name battle extends Object

enum combat_results {
	DEF_LOSS = 1,
	ATK_LOSS = 2,
	STALEMATE = -1,
}

var attacking_armies: Array[army] = []
var defending_armies: Array[army] = []

var atk_frontline_troops: Array[base_unit] = []
var atk_backline_troops: Array[base_unit] = []

var def_frontline_troops: Array[base_unit] = []
var def_backline_troops: Array[base_unit] = []

var is_stopped: bool = false

var day_ticks: int = 0

const INF_TICK: int = 1
const ART_TICK: int = 2
const CAV_TICK: int = 2

var MAX_MULTIPLE_TICK: int = -1

func _init() -> void:
	MAX_MULTIPLE_TICK = INF_TICK * ART_TICK * CAV_TICK

func get_combat_width() -> int:
	return 20

func add_atk_army(army_obj: army) -> void:
	attacking_armies.append(army_obj)
	refresh_attacking_lines(get_combat_width())

func add_def_army(army_obj: army) -> void:
	defending_armies.append(army_obj)
	refresh_defending_lines(get_combat_width())

func retreat_atk_army(index: int) -> void:
	attacking_armies.pop_at(index)
	# DO SOMETHING ELSE

func retreat_def_army(index: int) -> void:
	defending_armies.pop_at(index)
	# DO SOMETHING ELSE

func day_tick() -> void:
	if is_stopped:
		# Unit map will deal with battle
		return
	day_ticks += 1
	
	if day_ticks % INF_TICK:
		atk_inf_tick()
		def_inf_tick()
	if day_ticks % ART_TICK:
		atk_art_tick()
		def_art_tick()
	if day_ticks % CAV_TICK:
		atk_cav_tick()
		def_cav_tick()
	if day_ticks % MAX_MULTIPLE_TICK:
		day_ticks = 0
	
func atk_inf_tick() -> void:
	for index: int in atk_frontline_troops.size():
		var unit: base_unit = atk_frontline_troops[index]
		if unit is infantry:
			# Attack
			var sucess: bool = attack_frontline_defender(unit, index)
			if !sucess:
				sucess = attack_frontline_defender(unit, index - 1)
			if !sucess:
				sucess = attack_frontline_defender(unit, index + 1)

func def_inf_tick() -> void:
	for index: int in atk_frontline_troops.size():
		var unit: base_unit = atk_frontline_troops[index]
		if unit is infantry:
			# Attack
			var sucess: bool = attack_frontline_attacker(unit, index)
			if !sucess:
				sucess = attack_frontline_attacker(unit, index - 1)
			if !sucess:
				sucess = attack_frontline_attacker(unit, index + 1)

func atk_art_tick() -> void:
	pass

func def_art_tick() -> void:
	pass

func atk_cav_tick() -> void:
	pass

func def_cav_tick() -> void:
	pass

func attack_frontline_defender(atk: base_unit, index: int) -> bool:
	var size: int = def_frontline_troops.size()
	if size != 0 and index != -1 and index < size:
		var def: base_unit = def_frontline_troops[index]
		process_attack(atk, def)
		return true
	return false

func attack_frontline_attacker(atk: base_unit, index: int) -> bool:
	var size: int = atk_frontline_troops.size()
	if size != 0 and index != -1 and index < size:
		var def: base_unit = atk_frontline_troops[index]
		process_attack(atk, def)
		return true
	return false

func process_attack(atk: base_unit, def: base_unit) -> void:
	var result: combat_results = unit_battle(atk, def)
	if result == combat_results.DEF_LOSS:
		def.set_can_fight(false)
		refresh_defending_lines(get_combat_width())
	elif result == combat_results.ATK_LOSS:
		atk.set_can_fight(false)
		refresh_attacking_lines(get_combat_width())

#Attacker goes first which gives advantage but defender also gets bonuses
func unit_battle(attacker: base_unit, defender: base_unit) -> combat_results:
	var def_fire: int = defender.get_fire_damage()
	var def_shock: int = defender.get_shock_damage()
	
	attacker.add_battle_experience()
	defender.add_battle_experience()
	defender.remove_manpower(attacker.get_fire_damage())
	defender.remove_morale(attacker.get_shock_damage())
	
	
	if defender.get_manpower() == 0 or defender.get_morale() == 0:
		#kill unit
		return combat_results.DEF_LOSS
	
	attacker.remove_manpower(def_fire)
	attacker.remove_morale(def_shock)
	
	if attacker.get_manpower() == 0 or attacker.get_morale() == 0:
		#kill unit
		return combat_results.ATK_LOSS
	
	return combat_results.STALEMATE

func refresh_attacking_lines(combat_width: int) -> void:
	refresh_lines(attacking_armies, atk_frontline_troops, atk_backline_troops, combat_width)

func refresh_defending_lines(combat_width: int) -> void:
	refresh_lines(defending_armies, def_frontline_troops, def_backline_troops, combat_width)

func refresh_lines(armies: Array[army], frontline: Array[base_unit], backline: Array[base_unit], cw: int) -> void:
	# Clear to avoid having unit twice
	frontline.clear()
	backline.clear()
	frontline.resize(cw)
	backline.resize(cw)
	frontline.fill(null)
	backline.fill(null)
	
	var temp_front: Array = []
	var temp_back: Array = []
	for army_obj: army in armies:
		for unit: base_unit in army_obj.get_units():
			if unit.can_unit_fight():
				if unit.get_unit_range() == 1 and temp_front.size() < cw:
					temp_front.append(unit)
				elif unit.get_unit_range() == 2 and temp_back.size() < cw:
					temp_back.append(unit)
	
	# Centers the units
	@warning_ignore("integer_division")
	for i: int in range((cw - temp_front.size()) / 2, cw):
		frontline[i] = temp_front.pop_front()
	@warning_ignore("integer_division")
	for i: int in range((cw - temp_back.size()) / 2, cw):
		backline[i] = temp_back.pop_front()
	
	if frontline.size() == 0 and backline.size() == 0:
		#Battle is over
		is_stopped = true
		
