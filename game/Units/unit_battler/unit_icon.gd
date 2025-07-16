class_name unit_icon extends CharacterBody2D

enum Action {
	attack, # Will pursue route and attack units in range
	defend, # Will stay ground and attack units in range
	retreat, # Will retreat away in the safest direction will not attacking
	none # No affect
}

var moving_up: bool

var action: Action = Action.none
var id: int
var internal_unit: base_unit = null
var route: Array[Vector2] = []
var rotation_target: float = 0
var stuck_counter: int = 0
const STUCK_CONST: int = 10 # Number of ticks of collisions til unit reroutes 

var detected_friendlies: Array[unit_icon] = []
var detected_enemies: Array[unit_icon] = []

var terrain_map: TileMapLayer

# Utility
func get_number_of_friendlies() -> int:
	return (get_parent() as Brigade).get_size_of_units()

func get_number_of_enemies() -> int:
	return get_parent().get_parent().get_other_side_brigade(moving_up).get_size_of_units()

func assign_id(p_id: int) -> void:
	id = p_id

func assign_terrain_map(p_terrain_map: TileMapLayer) -> void:
	terrain_map = p_terrain_map

func assign_unit(p_unit: base_unit) -> void:
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

func set_range(unit_range: float) -> void:
	var size: float = 128 * (unit_range + 1)
	var new_shape: Shape2D = CircleShape2D.new()
	new_shape.radius = size
	$Area2D2/unit_range_collision_shape.shape = new_shape

func set_route(final_pos: Vector2) -> void:
	route.clear()
	route.append(final_pos)

func process(delta: float) -> void:
	detect_units()
	
	if (float(internal_unit.get_manpower()) / internal_unit.get_max_manpower()) < 0.1:
		set_retreat()
		create_route()
	
	if can_detect_enemy() and is_defending():
		var closest: unit_icon = get_closest_enemy_detected()
		fix_angle(closest.global_position)
		if enemy_is_advancing(closest):
			set_attack()
			annouce_attack()
	
	
	if !route.is_empty() and !can_unit_attack():
		advance_unit(delta)
	if route.is_empty() and !can_unit_attack(): # If no route and no attack then find a route
		create_route()

func day_tick() -> void:
	if can_unit_attack(): # Always attack if unit can
		attack_nearest_unit()

func detect_units() -> void:
	var detected_units: Array[unit_icon] = get_detected_units()
	detected_friendlies.clear()
	detected_enemies.clear()
	for unit: unit_icon in detected_units:
		if unit.is_friendly_with(id):
			detected_friendlies.push_back(unit)
		else:
			detected_enemies.push_back(unit)

func enemy_is_advancing(closest: unit_icon) -> bool:
	for unit: unit_icon in closest.get_units_in_range():
		if unit == self:
			return true
	return false

func annouce_attack() -> void:
	for unit: unit_icon in get_units_in_range():
		if unit.is_friendly_with(id):
			unit.set_attack()

func advance_unit(delta: float) -> void:
	var current_angle: float = $sprite.rotation_degrees
	var angle_to_target: float = angle_diff_deg(current_angle, rotation_target)

	var rotation_speed: int = 60 # Degrees per "second"
	var max_step: float = rotation_speed * delta

	if abs(angle_to_target) > 2:
		$sprite.rotation_degrees += clamp(angle_to_target, -max_step, max_step)
	
	# Rotation done
	var target: Vector2 = route.front()
	var unit_vect: Vector2 = (target - position).normalized()
	var mult: float = delta / get_physics_process_delta_time()
	var speed_vect: Vector2 = unit_vect * internal_unit.get_speed() * mult
	velocity = speed_vect
	var collided: bool = move_and_slide()
	
	if position.distance_to(target) < (speed_vect.length() / 2) or unit_vect.length() < 0.01:
		route.pop_front()
	
	if collided:
		stuck_counter += 1
		if stuck_counter > STUCK_CONST:
			stuck_counter = 0
			create_route()

func angle_diff_deg(a: float, b: float) -> float:
	var diff: float = fmod((b - a) + 180, 360) - 180
	return diff

func fix_angle(pos_to_face: Vector2) -> void:
	var pos: Vector2 = pos_to_face - global_position
	if pos.length() < 20:
		return
	var angle: float = -pos.angle_to(Vector2(0, -1))
	rotation_target = Utils.round(angle, 2)
	$sprite.rotation = Utils.round(angle, 2)

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
	internal_unit.attack_unit(internal_enemy)
	fix_angle(enemy.global_position)
	
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
	var scoring_func: Callable
	if internal_unit is calvary:
		scoring_func = score_target_location_cav
	else:
		scoring_func = score_target_location_inf
	# Checks staying still
	var start: Node2D = (self as Node2D)
	sampled_points.insert_element(start.global_position, scoring_func.call(start))
	
	var ray: Area2D = $sprite/CollisionRay
	for deg: int in range(0, 360, 15):
		ray.rotation_degrees = deg
		if is_path_clear():
			continue
		var target: Node2D = (ray.get_node("Target") as Node2D)
		var score: float = scoring_func.call(target)
		sampled_points.insert_element(target.global_position, score)
	route.clear()
	ray.rotation_degrees = 0
	if sampled_points.get_size() == 0:
		return
	var wv: weighted_value = sampled_points.pop_top_weighted_val()
	if wv.weight > 0:
		route.append(wv.val)
		rotation_target = (route.front() - global_position).angle() * 180 / PI + 90

func score_target_location_inf(target: Node2D) -> float:
	var score: float = 0
	var detected_units: Array[unit_icon] = get_unit_collisions($AwareRange/CollisionShape2D, target.global_transform)
	var units_in_range: Array[unit_icon] = get_unit_collisions($Area2D2/unit_range_collision_shape, target.global_transform)
	score += get_cohesion_score_inf(detected_units)
	score += get_outnumbed_score(detected_units)
	score += get_attacking_score(units_in_range)
	score += get_position_score(target.global_position, detected_units)
	# Add thing to just move to the closest enemy if attacking
	score += randf() - 0.5
	
	return score

func get_cohesion_score_inf(detected_units: Array[unit_icon]) -> float:
	var score: float = 0
	
	for unit: unit_icon in detected_units:
		if unit.is_friendly_with(id) and unit.internal_unit is infantry:
			score += 0.8
	#if is_leader(detected_units):
		#score += 1
	return score

func get_outnumbed_score(detected_units: Array[unit_icon]) -> float:
	var score: float = 0
	var f_num: int = 1
	var e_num: int = 0
	
	for unit: unit_icon in detected_units:
		if unit.is_friendly_with(id):
			f_num += 1
		else:
			e_num += 1
	if e_num != 0:
		var diff: int = f_num - e_num + 1
		score += diff / 4.0
		var ratio: float = (f_num + 1.0) / (e_num)
		var act_ratio: float = float(get_number_of_friendlies()) / get_number_of_enemies()
		var ratio_diff: float = ratio - act_ratio
		if ratio_diff < -0.2:
			score *= 2
	
	return score

func get_attacking_score(units_in_range: Array[unit_icon]) -> float:
	var score: float = 0
	var e_num: int = 0
	var f_num: int = 1
	for unit: unit_icon in units_in_range:
		if !unit.is_friendly_with(id):
			e_num += 1
		else:
			f_num += 1
	
	if e_num != 0:
		var diff: int = f_num - e_num + 1
		score += diff / 2.0 + 0.5
	if is_attacking():
		score *= 1.7
	if is_reteating():
		score *= -2
	return score

func get_position_score(target: Vector2, detected_units: Array[unit_icon]) -> float:
	var otherside_com: Vector2 = get_parent().get_parent().get_other_side_com(moving_up) # Calls unit battler to get center of mass
	var score: float = 0.0
	var tile_pos: Vector2i = terrain_map.local_to_map(target)
	
	if terrain_map.is_invalid(tile_pos):
		return -10000000
	
	var y_mov: float = target.y - position.y
	
	if abs(position.y - otherside_com.y) > 400: # If distance is large enough
		if position.y > otherside_com.y:
			score += max(min(-y_mov / 128, 2), -0.5) 
		else:
			score += max(min(y_mov / 128, 2), -0.5)
	
	var x_mov: float = target.x - position.x
	
	if abs(position.x - otherside_com.x) > 400: # If distance is large enough
		if position.x > otherside_com.x:
			score += max(min(-x_mov / 300, 0.7), -0.3) 
		else:
			score += max(min(x_mov / 300, 0.7), -0.3)
	
	if is_reteating():
		score *= -2
	
	if enemies_are_detected(detected_units):
		if terrain_map.is_hilly(tile_pos):
			score += 0.7
		elif terrain_map.is_forested(tile_pos):
			score += 0.4
		
		if is_attacking():
			var closest: Vector2 = get_closest_enemy(detected_units).position
			var current_dist: float = position.distance_to(closest)
			var this_dist: float = target.distance_to(closest)
			score += max(min((current_dist - this_dist) / 128, 2), -0.5) * 1.2
	
	return score

func is_leader(detected_units: Array[unit_icon]) -> bool:
	for unit: unit_icon in detected_units:
		if unit.is_friendly_with(id) and get_unit_experience() < unit.get_unit_experience(): # If less than not leader
			return false
	return true

func get_unit_experience() -> int:
	return internal_unit.experience

func score_target_location_cav(target: Node2D) -> float:
	var score: float = 0
	var detected_units: Array[unit_icon] = get_unit_collisions($AwareRange/CollisionShape2D, target.global_transform)
	var units_in_range: Array[unit_icon] = get_unit_collisions($Area2D2/unit_range_collision_shape, target.global_transform)
	score += get_cohesion_score_cav(detected_units)
	score += get_outnumbed_score(detected_units)
	score += get_attacking_score(units_in_range)
	score += get_position_score(target.global_position, detected_units)
	# Flanking function / Searching function
	score += randf() - 0.5
	
	return score

func get_cohesion_score_cav(detected_units: Array[unit_icon]) -> float:
	var score: float = 0
	
	for unit: unit_icon in detected_units:
		if unit.is_friendly_with(id) and unit.internal_unit is calvary:
			score += 0.8
	
	return score

func enemies_are_detected(detected_units: Array[unit_icon]) -> bool:
	for unit: unit_icon in detected_units:
		if !unit.is_friendly_with(id):
			return true
	return false

func has_route() -> bool:
	return !route.is_empty()

# == Collisions ==

func get_units_in_range() -> Array[unit_icon]:
	return get_unit_collisions($Area2D2/unit_range_collision_shape)

func get_detected_units() -> Array[unit_icon]:
	return get_unit_collisions($AwareRange/CollisionShape2D)

func get_future_collisions() -> Array[unit_icon]:
	return get_unit_collisions($sprite/CollisionRay)

func get_unit_collisions(collision_shape: CollisionShape2D, transformation: Transform2D = collision_shape.global_transform) -> Array[unit_icon]:
	var space_state: PhysicsDirectSpaceState2D = get_world_2d().direct_space_state
	var shape: Shape2D = collision_shape.shape
	var ex_transform: Transform2D = transformation  # Includes rotation and position
	
	var arg: PhysicsShapeQueryParameters2D = PhysicsShapeQueryParameters2D.new()
	arg.shape = shape
	arg.transform = ex_transform
	arg.margin = 0.01
	arg.exclude = [self]
	var results: Array[Dictionary] = space_state.intersect_shape(arg, 10)
	
	var toReturn: Array[unit_icon] = []
	for hit: Dictionary in results:
		if hit.collider is unit_icon:
			toReturn.push_back(hit.collider)
	return toReturn

#func get_unit_collisions(area: Area2D) -> Array[unit_icon]:
	#var toReturn: Array[unit_icon] = []
	#var collisons: Array[Node2D] = area.get_overlapping_bodies()
	#for collision: Node2D in collisons:
		#if collision is unit_icon:
			#toReturn.push_back(collision)
	#return toReturn

func will_collide_with_other_unit() -> bool:
	return get_future_collisions().size() != 0

func is_path_clear() -> bool:
	var space_state: PhysicsDirectSpaceState2D = get_world_2d().direct_space_state
	var shape_node: CollisionShape2D = $sprite/CollisionRay/CollisionShape2D
	var shape: Shape2D = shape_node.shape
	var ex_transform: Transform2D = shape_node.global_transform  # Includes rotation and position
	
	var arg: PhysicsShapeQueryParameters2D = PhysicsShapeQueryParameters2D.new()
	arg.shape = shape
	arg.transform = ex_transform
	arg.margin = 0.01
	arg.exclude = [self]
	var results: Array[Dictionary] = space_state.intersect_shape(arg, 4)

	for hit: Dictionary in results:
		if hit.collider is unit_icon:
			return true
	return false

func update_labels() -> void:
	$UI/MoralBar.value = internal_unit.get_morale()
	($UI/ManpowerLabel as Label).text = str(internal_unit.get_manpower())

func can_unit_fight() -> bool:
	return internal_unit.can_unit_fight()
