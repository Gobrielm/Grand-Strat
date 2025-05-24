class_name client_army extends army

func _init(p_player_id: int, p_location: Vector2i, p_army_id: int) -> void:
	player_id = p_player_id
	location = p_location
	army_id = p_army_id


##Unit array = [manpower, morale, experience, org.get_organization(), dest]
func update_stats(army_info: Array, units_info: Array) -> void:
	units.clear()
	for unit_info: Array in units_info:
		var unit: base_unit = base_unit.new()
		unit.update_stats(unit_info)
		add_unit(unit)
	if army_info[4]:
		route = [army_info[4]]
