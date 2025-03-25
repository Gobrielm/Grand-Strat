class_name base_unit extends Node

const speed_mult_hilly = 0.65
const speed_mult_other_unit = 0.45

func _init(new_location: Vector2i, new_player_id: int):
	location = new_location
	player_id = new_player_id

func convert_to_client_array() -> Array:
	return [player_id, location, get_destination(), manpower, morale, experience]

static func get_speed_mult(terrain_num: int):
	if terrain_num == 0:
		return speed_mult_hilly
	else:
		return 1

static func get_cost() -> int:
	return 0

#Where the unit is
var location: Vector2i

func get_location() -> Vector2i:
	return location

func set_location(new_location: Vector2i):
	location = new_location

var player_id: int

func get_player_id() -> int:
	return player_id

#Max manpower the unit has
var max_manpower: int

func get_max_manpower() -> int:
	return max_manpower

#How much manpower the unit has
var manpower: int

func get_manpower() -> int:
	return manpower

func add_manpower(amount: int) -> int:
	var amount_added = min(amount, max_manpower - manpower)
	manpower += amount_added
	return amount_added

func remove_manpower(amount: int):
	manpower -= amount
	if manpower < 0:
		manpower = 0

const max_morale = 100
#The desire for the unit to fight
var morale: int

func get_morale() -> int:
	return morale

func add_morale(amount: int):
	morale += amount
	if morale > 100:
		morale = 100

func remove_morale(amount: int):
	morale -= round(amount / float(cohesion) * 20)
	if morale < 0:
		morale = 0

#How fast a unit can move
var speed: int

func get_speed() -> int:
	return speed

var unit_range: int

func get_unit_range() -> int:
	return unit_range

#The route the unit takes if travelling
var route: Array

func set_route(new_route: Array):
	route = new_route

func get_next_location(index = 0) -> Vector2i:
	if route.is_empty() or index >= route.size():
		return location
	return route[index]

func pop_next_location() -> Vector2i:
	return route.pop_front()

func is_route_empty() -> bool:
	return route.is_empty()

func get_destination():
	if route.is_empty():
		return null
	return route.back()

func stop():
	route = []

#The progress unit has to travel
var progress: float

func update_progress(num: float):
	progress += num

func reset_progress():
	progress = 0

func ready_to_move(progress_needed: float) -> bool:
	if progress_needed < progress:
		reset_progress()
		return true
	return false

#TODO: Add more variables
func get_shock_damage() -> int:
	var expierence_mult = (float(experience) / 1000) + 1
	return round((shock / 200 * expierence_mult) * (manpower + 100))
#TODO: Add more variables
func get_fire_damage() -> int:
	var expierence_mult = (float(experience) / 1000) + 1
	return round((firepower / 200 * expierence_mult) * (manpower + 100))

#The amount of supplies the unit has
var organization

#The morale damage a unit does
var shock: float

#The general damage
var firepower: float

#Morale defense, defense in general
var cohesion: int
#The disipline and skill of the unit
var experience: int

func add_experience(multiple = 1.0):
	experience += round(float(experience_gain) * multiple)

func add_battle_experience(multiple = 1.0):
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

func set_atlas_coord(atlas_coords: Vector2i):
	specific_type = atlas_coords.x
	combat_arm = atlas_coords.y

func get_atlas_coord() -> Vector2i:
	return Vector2i(specific_type, combat_arm)
