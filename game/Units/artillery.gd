class_name artillery extends base_unit

static func get_cost() -> int:
	return 1000

func _init() -> void:
	
	max_manpower = 200
	manpower = max_manpower
	morale = 100
	
	var supply_neeeded: Dictionary[int, int] = {}
	supply_neeeded[terminal_map.get_cargo_type("grain")] = 2
	supply_neeeded[terminal_map.get_cargo_type("artillery")] = 1
	org = organization.new(supply_neeeded)
	
	speed = 10
	unit_range = 2
	shock = 40
	firepower = 100
	cohesion = 15
	experience_gain = 2
	battle_multiple = 10
	
	experience = 0
	combat_arm = 2

func _to_string() -> String:
	return "Artillery"

static func toString() -> String:
	return "Artillery"
