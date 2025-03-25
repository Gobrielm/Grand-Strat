class_name artillery extends base_unit

static func get_cost() -> int:
	return 1000

func _init(new_location: Vector2i, new_player_id: int):
	super._init(new_location, new_player_id)
	
	max_manpower = 200
	manpower = max_manpower
	morale = 100
	
	organization = null
	speed = 10
	unit_range = 2
	shock = 40
	firepower = 100
	cohesion = 15
	experience_gain = 2
	battle_multiple = 10
	
	experience = 0
	combat_arm = 2

func _to_string():
	return "Artillery"

static func toString() -> String:
	return "Artillery"
