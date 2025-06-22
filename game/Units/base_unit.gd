class_name base_unit extends RefCounted

func _init() -> void:
	pass

##return [manpower, morale, experience, org.convert_to_client_array(), atlas_inof]
func convert_to_client_array() -> Array:
	#TODO: Add more stuff
	return [manpower, morale, experience, org.convert_to_client_array(), get_atlas_coord()]

##return [manpower, morale, experience, org.get_organization(), atlas_inof]
func convert_to_client_array_for_army() -> Array:
	#TODO: Add more stuff
	return [manpower, morale, experience, org.get_organization()]

func get_client_dict_for_army() -> Dictionary:
	return {
		"manpower": manpower, 
		"morale": morale, "experience": experience, 
		"org": org.get_organization()
	}

func update_stats(unit_info: Array) -> void:
	manpower = unit_info[0]
	morale = unit_info[1]
	experience = unit_info[2]
	org = organization.create_with_client_array(unit_info[3])
	specific_type = unit_info[4].x
	combat_arm = unit_info[4].y

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

func get_level() -> int:
	var experience_array: Array[int] = [-1, 200, 500, 1000, 2000, 5000]
	for index: int in range(5, -1, -1):
		if experience > experience_array[index]:
			return index
	return 0

func get_level_as_string() -> String:
	var experience_names: Array[String] = ["Inexperienced", "Trained", "Experienced", "Expert", "Verteran", "Elite"]
	return experience_names[get_level()]

#The rate at which a unit gains experience
var experience_gain: int
#The bonus to experience_gain when in battle
var battle_multiple: int

#The specification, infantry, cav, ect. the y atlas
#Infantry, Calvary, officer, engineer, artillery, 
var combat_arm: int
#The actual type line infantry, mechanized infantry, ect. the x atlas
var specific_type: int

var can_fight: bool = true

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

func set_atlas_coord(atlas_coords: Vector2i) -> void:
	specific_type = atlas_coords.x
	combat_arm = atlas_coords.y

func get_atlas_coord() -> Vector2i:
	return Vector2i(specific_type, combat_arm)

func can_unit_fight() -> bool:
	return can_fight

func set_can_fight(_can_fight: bool) -> void:
	can_fight = _can_fight
