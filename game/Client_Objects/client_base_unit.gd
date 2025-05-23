extends base_unit

func _init():
	manpower = 0
	morale = 0
	experience = 0
	
##[manpower, morale, experience, org.get_organization()]
func update_stats(unit_info: Array):
	manpower = unit_info[0]
	morale = unit_info[1]
	experience = unit_info[2]

func convert_to_client_array() -> Array:
	return [manpower, morale, experience]
