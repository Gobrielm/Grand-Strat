class_name client_army extends army

const client_unit: GDScript = preload("res://Client_Objects/client_base_unit.gd")

func _init(p_player_id: int, p_location: Vector2i) -> void:
	player_id = p_player_id
	location = p_location


##Unit array = [manpower, morale, experience, org.get_organization(), dest]
func update_stats(army_info: Array, units_info: Array) -> void:
	for unit_info: Array in units_info:
		var unit: base_unit = client_unit.new()
		unit.update_stats(unit_info)
		add_unit(unit)
	route = [army_info[4]]
