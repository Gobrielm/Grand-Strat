class_name unit_icon extends Node2D

enum Action {
	attack, # Will pursue route and attack units in range
	defend, # Will stay ground and attack units in range
	retreat, # Will retreat away in the safest direction will not attacking
	none # No affect
}

var action: Action = Action.none
var id: int
var area_range: Area2D
var aware_range: Area2D
var unit_range_shape: CollisionShape2D
var sprite: Sprite2D
var internal_unit: base_unit = null
var route: Array[Vector2] = []

func assign_id(p_id: int) -> void:
	id = p_id

func assign_unit(p_unit: base_unit) -> void:
	area_range = get_node("Area2D2")
	unit_range_shape = get_node("Area2D2/unit_range_collision_shape")
	aware_range = get_node("AwareRange")
	sprite = get_node("sprite")
	internal_unit = p_unit
	set_range(p_unit.get_unit_range())
	update_labels()

func set_attack() -> void:
	action = Action.attack

func is_attacking() -> bool:
	return action == Action.attack

func set_defend() -> void:
	action = Action.defend

func is_defending() -> bool:
	return action == Action.defend

func set_retreat() -> void:
	action = Action.retreat

func is_reteating() -> bool:
	return action == Action.retreat

func is_friendly_with(p_id: int) -> bool:
	return p_id == id

func set_range(unit_range: int) -> void:
	var size: int = 128 * (unit_range + 1)
	var new_shape: Shape2D = RectangleShape2D.new()
	new_shape.size = Vector2(size, size)
	unit_range_shape.shape = new_shape

func set_route(final_pos: Vector2) -> void:
	route.clear()
	route.append(final_pos)

func day_tick() -> void:
	if can_detect_enemy() and is_defending():
		var closest: unit_icon = get_closest_enemy_detected()
		fix_angle(closest.global_position)
		return
	
	if !route.is_empty() and !can_unit_attack():
		advance_unit()
	elif can_unit_attack():
		attack_nearest_unit()

func advance_unit() -> void:
	var target: Vector2 = route.front()
	var unit_vect: Vector2 = (target - position).normalized()
	var speed_vect: Vector2 = unit_vect * internal_unit.get_speed()
	position.x += speed_vect.x
	position.y += speed_vect.y
	
	if position.distance_to(target) < (speed_vect.length() / 2):
		
		fix_angle(route.pop_front())
		return
	fix_angle(route.front())

func fix_angle(pos_to_face: Vector2) -> void:
	var pos: Vector2 = pos_to_face - global_position
	if pos.length() < 20:
		return
	var angle: float = -pos.angle_to(Vector2(0, -1))
	sprite.rotation = Utils.round(angle, 2)

func can_detect_enemy() -> bool:
	var units: Array[unit_icon] = get_detected_units()
	return units.size() != 0

func can_unit_attack() -> bool:
	var units: Array[unit_icon] = get_units_in_range()
	return units.size() != 0

func attack_nearest_unit() -> void:
	var enemy: unit_icon = get_closest_enemy_in_range()
	var internal_enemy: base_unit = enemy.internal_unit
	internal_enemy.remove_manpower(internal_unit.get_fire_damage())
	internal_enemy.remove_morale(internal_unit.get_shock_damage())
	fix_angle(enemy.global_position)
	
	if internal_enemy.can_unit_fight():
		internal_unit.remove_manpower(internal_enemy.get_fire_damage())
		internal_unit.remove_morale(internal_enemy.get_shock_damage())
	
	enemy.update_labels()
	update_labels()

func get_closest_enemy_in_range() -> unit_icon:
	return get_closest_enemy(get_units_in_range())

func get_closest_enemy_detected() -> unit_icon:
	return get_closest_enemy(get_detected_units())

func get_closest_enemy(units: Array[unit_icon]) -> unit_icon:
	var closest: unit_icon = null
	var dist: float = 0
	for unit: unit_icon in units:
		var temp_dist: float = unit.position.distance_to(position)
		if temp_dist < dist or closest == null:
			closest = unit
			dist = temp_dist
	return closest

func has_route() -> bool:
	return !route.is_empty()

func get_units_in_range() -> Array[unit_icon]:
	return get_unit_collisions(area_range)

func get_detected_units() -> Array[unit_icon]:
	return get_unit_collisions(aware_range)

func get_unit_collisions(area: Area2D) -> Array[unit_icon]:
	var toReturn: Array[unit_icon] = []
	var collisons: Array[Area2D] = area.get_overlapping_areas()
	for collision: Area2D in collisons:
		var parent: Variant = collision.get_parent() 
		if collision.name == "MainBody" and parent is unit_icon and !parent.is_friendly_with(id):
			toReturn.push_back(parent as unit_icon) 
	return toReturn

func update_labels() -> void:
	$UI/MoralBar.value = internal_unit.get_morale()
	($UI/ManpowerLabel as Label).text = str(internal_unit.get_manpower())

func can_unit_fight() -> bool:
	return internal_unit.can_unit_fight()
