class_name unit_icon extends Node2D

enum Action {
	attack, # Will pursue route and attack units in range
	defend, # Will stay ground and attack units in range
	retreat, # Will retreat away in the safest direction will not attacking
	none # No affect
}

var moving_up: bool

var action: Action = Action.none
var id: int
var attack_range: Area2D
var aware_range: Area2D
var unit_range_shape: CollisionShape2D
var sprite: Sprite2D
var internal_unit: base_unit = null
var route: Array[Vector2] = []

var detected_friendlies: Array[unit_icon] = []
var detected_enemies: Array[unit_icon] = []

var terrain_map: TileMapLayer

func assign_id(p_id: int) -> void:
	id = p_id

func assign_terrain_map(p_terrain_map: TileMapLayer) -> void:
	terrain_map = p_terrain_map

func assign_unit(p_unit: base_unit) -> void:
	attack_range = get_node("Area2D2")
	unit_range_shape = get_node("Area2D2/unit_range_collision_shape")
	aware_range = get_node("AwareRange")
	sprite = get_node("sprite")
	internal_unit = p_unit
	set_range(p_unit.get_unit_range())
	update_labels()

func set_up(p_position: Vector2) -> void:
	position = p_position
	moving_up = (position.y > 0)

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
	var new_shape: Shape2D = CircleShape2D.new()
	new_shape.radius = size
	unit_range_shape.shape = new_shape

func set_route(final_pos: Vector2) -> void:
	route.clear()
	route.append(final_pos)

func day_tick() -> void:
	detect_units()
	
	if can_detect_enemy() and is_defending():
		var closest: unit_icon = get_closest_enemy_detected()
		fix_angle(closest.global_position)
		return
	
	if can_unit_attack(): # Always attack if unit can
		attack_nearest_unit()
	elif !route.is_empty(): # If can't attack then move
		advance_unit()
	if route.is_empty() and !can_unit_attack(): # If no route and no attack then find a route
		create_route()

func detect_units() -> void:
	var detected_units: Array[unit_icon] = get_detected_units()
	detected_friendlies.clear()
	detected_enemies.clear()
	for unit: unit_icon in detected_units:
		if unit.is_friendly_with(id):
			detected_friendlies.push_back(unit)
		else:
			detected_enemies.push_back(unit)

func advance_unit() -> void:
	var target: Vector2 = route.front()
	var unit_vect: Vector2 = (target - position).normalized()
	var speed_vect: Vector2 = unit_vect * internal_unit.get_speed()
	position.x += speed_vect.x
	position.y += speed_vect.y
	if is_colliding_with_other_unit():
		route.clear()
		position.x -= speed_vect.x
		position.y -= speed_vect.y
		return
	
	if position.distance_to(target) < (speed_vect.length() / 2):
		fix_angle(route.pop_front())
		return
	fix_angle(route.front())

func will_collide_with_other_unit() -> bool:
	return is_shape_colliding_with_unit($sprite/CollisionRay/CollisionShape2D.shape)

func is_colliding_with_other_unit() -> bool:
	return is_shape_colliding_with_unit($MainBody/CollisionShape2D.shape)

func is_shape_colliding_with_unit(shape: Shape2D) -> bool:
	var space_state: PhysicsDirectSpaceState2D = get_world_2d().direct_space_state
	var arg: PhysicsShapeQueryParameters2D = PhysicsShapeQueryParameters2D.new()
	arg.shape = shape
	arg.transform = transform
	arg.collide_with_areas = true
	arg.collide_with_bodies = false
	arg.margin = 0.01
	arg.exclude = [$MainBody, $Area2D2, $AwareRange, $sprite/CollisionRay]
	var results: Array[Dictionary] = space_state.intersect_shape(arg, 20)
	for result: Dictionary in results:
		if is_result_main_body(result):
			return true
	return false

func is_result_main_body(result: Dictionary) -> bool:
	var collider: Area2D = result.collider
	return is_area_unit_mainbody(collider)

func fix_angle(pos_to_face: Vector2) -> void:
	var pos: Vector2 = pos_to_face - global_position
	if pos.length() < 20:
		return
	var angle: float = -pos.angle_to(Vector2(0, -1))
	sprite.rotation = Utils.round(angle, 2)

func can_detect_enemy() -> bool:
	return detected_enemies.size() != 0

func can_unit_attack() -> bool:
	var units: Array[unit_icon] = get_units_in_range()
	for unit: unit_icon in units:
		if !unit.is_friendly_with(id):
			return true
	return false

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
	return get_closest_enemy(detected_enemies)

func get_closest_enemy(units: Array[unit_icon]) -> unit_icon:
	var closest: unit_icon = null
	var dist: float = 0
	for unit: unit_icon in units:
		if unit.is_friendly_with(id):
			continue
		var temp_dist: float = unit.position.distance_to(position)
		if temp_dist < dist or closest == null:
			closest = unit
			dist = temp_dist
	return closest

func create_route() -> void:
	var sampled_points: priority_queue = priority_queue.new()
	var ray: Area2D = $sprite/CollisionRay
	for deg: int in range(0, 360, 15):
		ray.rotation_degrees = deg
		if will_collide_with_other_unit():
			continue
		var target: Node2D = (ray.get_node("Target") as Node2D)
		var score: float = score_target_location(target)
		sampled_points.insert_element(target.global_position, score)
	route.clear()
	ray.rotation_degrees = 0
	if sampled_points.get_size() == 0:
		return
	route.append(sampled_points.pop_top())

func score_target_location(target: Node2D) -> float:
	var score: float = 0
	aware_range.position = target.position
	attack_range.position = target.position
	var detected_units: Array[unit_icon] = get_detected_units()
	var units_in_range: Array[unit_icon] = get_units_in_range()
	score += get_cohesion_score(detected_units)
	score += get_attacking_score(units_in_range)
	score += get_positon_score(target.global_position, detected_units)
	
	aware_range.position = Vector2(0, 0)
	attack_range.position = Vector2(0, 0)
	return score

func get_cohesion_score(detected_units: Array[unit_icon]) -> float:
	var score: float = 0
	
	for unit: unit_icon in detected_units:
		if unit.is_friendly_with(id):
			score += 1.2
		else:
			score -= 1
	
	return score

func get_attacking_score(units_in_range: Array[unit_icon]) -> float:
	var score: float = 0
	var num: int = 0
	for unit: unit_icon in units_in_range:
		if !unit.is_friendly_with(id):
			num += 1
	
	if num == 1:
		score += 3
	elif num == 2:
		score += 0.3
	else:
		score -= 1
	
	return score

func get_positon_score(target: Vector2, detected_units: Array[unit_icon]) -> float:
	var score: float = 0.0
	var tile_pos: Vector2i = terrain_map.local_to_map(target)
	
	if terrain_map.is_invalid(tile_pos):
		return -10000000
	
	var y_mov: float = target.y - position.y
	if moving_up:
		score += max(min(-y_mov / 256, 1), -0.5)
	else:
		score += max(min(y_mov / 256, 1), -0.5)
	
	
	if detected_units.size() != 0: # Only count terrain score if enemies are nearby
		if terrain_map.is_hilly(tile_pos):
			score += 0.7
		elif terrain_map.is_forested(tile_pos):
			score += 0.4
	
	return score

func has_route() -> bool:
	return !route.is_empty()

func get_units_in_range() -> Array[unit_icon]:
	return get_unit_collisions(attack_range)

func get_detected_units() -> Array[unit_icon]:
	return get_unit_collisions(aware_range)

func get_unit_collisions(area: Area2D) -> Array[unit_icon]:
	var toReturn: Array[unit_icon] = []
	var collisons: Array[Area2D] = area.get_overlapping_areas()
	for collision: Area2D in collisons:
		var parent: Variant = collision.get_parent() 
		if is_area_unit_mainbody(collision):
			toReturn.push_back(parent as unit_icon) 
	return toReturn

func is_area_unit_mainbody(area: Area2D) -> bool:
	var parent: Variant = area.get_parent() 
	return area.name == "MainBody" and parent is unit_icon

func update_labels() -> void:
	$UI/MoralBar.value = internal_unit.get_morale()
	($UI/ManpowerLabel as Label).text = str(internal_unit.get_manpower())

func can_unit_fight() -> bool:
	return internal_unit.can_unit_fight()
