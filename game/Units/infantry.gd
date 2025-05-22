class_name infantry extends base_unit

static func get_cost() -> int:
	return 300

func _init() -> void:
	max_manpower = 1000
	manpower = 1000
	morale = 100
	
	var supply_neeeded: Dictionary[int, int] = {}
	supply_neeeded[terminal_map.get_cargo_type("grain")] = 2
	supply_neeeded[terminal_map.get_cargo_type("guns")] = 1
	org = organization.new(supply_neeeded)
	
	speed = 30
	unit_range = 1
	shock = 20
	firepower = 50
	cohesion = 100
	experience_gain = 5
	battle_multiple = 5
	
	experience = 0
	combat_arm = 0

func _to_string() -> String:
	return "Infantry"

static func toString() -> String:
	return "Infantry"
