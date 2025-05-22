extends base_unit

func _init(new_atlas_coords: Vector2i):
	set_atlas_coord(new_atlas_coords)
	manpower = 0
	morale = 0
	experience = 0
	

func update_stats(unit_info: Array):
	manpower = unit_info[1]
	morale = unit_info[2]
	experience = unit_info[3]

func convert_to_client_array() -> Array:
	return [manpower, morale, experience]
