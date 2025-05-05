class_name officer extends base_unit

var experience_aura_boost: float

static func get_cost() -> int:
	return 500

func add_experience(multiple: float = 1.0) -> void:
	super.add_experience(multiple)
	recalculate_experience_aura_boost()

func add_battle_experience(multiple: float = 1.0) -> void:
	super.add_battle_experience(multiple)
	recalculate_experience_aura_boost()

func _init(new_location: Vector2i, new_player_id: int) -> void:
	super._init(new_location, new_player_id)
	
	max_manpower = 50
	manpower = max_manpower
	morale = 100
	
	speed = 80
	unit_range = 1
	shock = 5
	firepower = 5
	cohesion = 5
	experience_gain = 1
	battle_multiple = 40
	
	experience_aura_boost = 1.0
	
	experience = 5000
	combat_arm = 4

func get_aura_boost() -> float:
	return experience_aura_boost

func recalculate_experience_aura_boost() -> void:
	experience_aura_boost = Utils.round(float(experience) / 10000 + 1, 2)

func _to_string() -> String:
	return "Officer"

static func toString() -> String:
	return "Officer"
