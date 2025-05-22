class_name engineer extends base_unit

static func get_cost() -> int:
	return 700

func _init() -> void:

	max_manpower = 100
	manpower = max_manpower
	morale = 100
	
	speed = 50
	unit_range = 1
	shock = 0
	firepower = 0
	cohesion = 10
	experience_gain = 10
	battle_multiple = 2
	
	experience = 0
	combat_arm = 3

func _to_string() -> String:
	return "Engineer"

static func toString() -> String:
	return "Engineer"
