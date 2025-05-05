class_name calvary extends base_unit

static func get_cost() -> int:
	return 700

func _init(new_location: Vector2i, new_player_id: int) -> void:
	super._init(new_location, new_player_id)
	
	max_manpower = 600
	manpower = max_manpower
	morale = 100
	
	speed = 80
	unit_range = 1
	shock = 100
	firepower = 25
	cohesion = 40
	experience_gain = 3
	battle_multiple = 10
	
	experience = 0
	combat_arm = 1

func _to_string() -> String:
	return "Calvary"

static func toString() -> String:
	return "Calvary"
