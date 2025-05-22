class_name base_unit extends Node

func _init() -> void:
	pass

func convert_to_client_array() -> Array:
	#TODO: Add more stuff
	return [manpower, morale, experience, org.get_organization()]

static func get_cost() -> int:
	return 0

#Max manpower the unit has
var max_manpower: int

#How much manpower the unit has
var manpower: int

#Max morale
const max_morale: int = 100
#The desire for the unit to fight
var morale: int

#How fast a unit can move
var speed: int

var unit_range: int

#The amount of supplies the unit has
var org: organization

#The morale damage a unit does
var shock: float

#The general damage
var firepower: float

#Morale defense, defense in general
var cohesion: int
#The disipline and skill of the unit
var experience: int

#The rate at which a unit gains experience
#level 0 - inexperienced, 0 - 200
#level 1 - trained,       200 - 500
#level 2 - experienced,   500 - 1000
#level 3 - expert,        1000 - 2000
#level 4 - verteran,      2000 - 5000
#level 5 - elite,         5000 - ∞
var experience_gain: int
#The bonus to experience_gain when in battle
var battle_multiple: int

#The specification, infantry, cav, ect. the y atlas
#Infantry, Calvary, officer, engineer, artillery, 
var combat_arm: int
#The actual type line infantry, mechanized infantry, ect. the x atlas
var specific_type: int

func get_max_manpower() -> int:
	return max_manpower

func get_manpower() -> int:
	return manpower

func add_manpower(amount: int) -> int:
	var amount_added: int = min(amount, max_manpower - manpower)
	manpower += amount_added
	return amount_added

func remove_manpower(amount: int) -> void:
	manpower -= amount
	if manpower < 0:
		manpower = 0

func get_morale() -> int:
	return morale

func add_morale(amount: int) -> void:
	morale += amount
	if morale > 100:
		morale = 100

func remove_morale(amount: int) -> void:
	morale -= round(amount / float(cohesion) * 20)
	if morale < 0:
		morale = 0

func get_speed() -> int:
	return speed

func get_unit_range() -> int:
	return unit_range

#TODO: Add more variables
func get_shock_damage() -> int:
	var expierence_mult: float = (float(experience) / 1000) + 1
	return round((shock / 200 * expierence_mult) * (manpower + 100))
#TODO: Add more variables
func get_fire_damage() -> int:
	var expierence_mult: float = (float(experience) / 1000) + 1
	return round((firepower / 200 * expierence_mult) * (manpower + 100))

func get_organization_object() -> organization:
	return org

func use_supplies() -> void:
	org.use_supplies()

func add_experience(multiple: float = 1.0) -> void:
	experience += round(float(experience_gain) * multiple)

func add_battle_experience(multiple: float = 1.0) -> void:
	experience += round(float(experience_gain) * float(battle_multiple) * multiple)

func get_level() -> int:
	if experience > 5000:
		return 5
	elif experience > 2000:
		return 4
	elif experience > 1000:
		return 3
	elif experience > 500:
		return 2
	elif experience > 200:
		return 1
	else:
		return 0

func set_atlas_coord(atlas_coords: Vector2i) -> void:
	specific_type = atlas_coords.x
	combat_arm = atlas_coords.y

func get_atlas_coord() -> Vector2i:
	return Vector2i(specific_type, combat_arm)
