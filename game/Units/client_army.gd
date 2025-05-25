class_name client_army extends army

func _init(p_player_id: int, p_location: Vector2i, p_army_id: int) -> void:
	player_id = p_player_id
	location = p_location
	army_id = p_army_id


##Unit array = [manpower, morale, experience, org.get_organization(), dest]
func update_stats(army_info: Array) -> void:
	if army_info[4]:
		route = [army_info[4]]

func refresh_unit(index: int, unit_array: Array) -> void:
	var base_units: Array[base_unit] = get_units()
	var unit: base_unit
	if base_units.size() <= index:
		unit = base_unit.new()
	else:
		unit = base_units[index]
	unit.update_stats(unit_array)
