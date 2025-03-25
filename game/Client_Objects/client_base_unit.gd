extends base_unit

var destination

func get_destination():
	return destination

func _init(new_location: Vector2i, new_player_id: int, new_atlas_coords: Vector2i):
	location = new_location
	player_id = new_player_id
	set_atlas_coord(new_atlas_coords)
	manpower = 0
	morale = 0
	experience = 0
	

func update_stats(unit_info: Array):
	#player_id = unit_info[0]
	location = unit_info[1]
	destination = unit_info[2]
	manpower = unit_info[3]
	morale = unit_info[4]
	experience = unit_info[5]

func convert_to_client_array() -> Array:
	return [player_id, location, destination, manpower, morale, experience]
