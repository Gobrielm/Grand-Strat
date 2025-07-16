class_name calvary extends base_unit

static func get_cost() -> int:
	return 700

func _init() -> void:
	
	max_manpower = 600
	manpower = max_manpower
	morale = 100
	
	var supply_neeeded: Dictionary[int, int] = {}
	supply_neeeded[CargoInfo.get_instance().get_cargo_type("grain")] = 2
	supply_neeeded[CargoInfo.get_instance().get_cargo_type("guns")] = 1
	org = organization.new(supply_neeeded)
	
	speed = 80
	unit_range = 0.5
	shock = 60
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
