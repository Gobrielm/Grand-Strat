class_name unit_icon extends Sprite2D

var id: int
var area_range: Area2D
var unit_range_shape: CollisionShape2D
var internal_unit: base_unit = null
var route: Array[Vector2] = []

func assign_id(p_id: int) -> void:
	id = p_id

func assign_unit(p_unit: base_unit) -> void:
	area_range = get_node("Area2D2")
	unit_range_shape = get_node("Area2D2/unit_range_collision_shape")
	internal_unit = p_unit
	set_range(p_unit.get_unit_range())

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

#func bfs_to_dest(start: Vector2, dest: Vector2) -> Array[Vector2]:
	#var tile_to_prev: Dictionary[Vector2i, Vector2i] = {}
	#var queue: Array[Vector2i] = [unit_locations.local_to_map(start)]
	#var end: Vector2i = unit_locations.local_to_map(dest)
	#var found: bool = false
	#
	#while !queue.is_empty() and !found:
		#var curr: Vector2i = queue.pop_front()
		#for tile: Vector2i in unit_locations.get_surrounding_cells(curr):
			#if unit_locations.is_valid(tile): # Is Empty
				#queue.push_back(tile)
				#tile_to_prev[tile] = curr
				#if tile == end:
					#found = true
					#break
	#
	#var toReturn: Array[Vector2] = []
	#
	#if found:
		#var temp: Vector2i = end
		#toReturn.push_back(temp)
		#while tile_to_prev.has(temp):
			#temp = tile_to_prev[temp]
			#toReturn.push_back(temp)
	#
	#return toReturn

func day_tick() -> void:
	if !route.is_empty() and !can_unit_attack():
		move_unit()
	elif can_unit_attack():
		pass

func move_unit() -> void:
	var target: Vector2 = route.front()
	var unit_vect: Vector2 = (target - position).normalized()
	var speed_vect: Vector2 = unit_vect * internal_unit.get_speed()
	#erase_old_position_with_unit_locations()
	position.x += speed_vect.x
	position.y += speed_vect.y
	#TODO: Check if tile is free here and reroute if needed
	#set_position_with_unit_locations()
	if position.distance_to(target) < (speed_vect.length() / 2):
		route.pop_front()

func can_unit_attack() -> bool:
	var units: Array[unit_icon] = get_unit_collisions()
	return units.size() != 0

func has_route() -> bool:
	return !route.is_empty()

func get_unit_collisions() -> Array[unit_icon]:
	var toReturn: Array[unit_icon] = []
	var collisons: Array[Area2D] = area_range.get_overlapping_areas()
	for collision: Area2D in collisons:
		var parent: Variant = collision.get_parent() 
		if collision.name == "MainBody" and parent is unit_icon and !parent.is_friendly_with(id):
			toReturn.push_back(parent as unit_icon) 
	return toReturn

#func erase_old_position_with_unit_locations() -> void:
	#var tile: Vector2i = unit_locations.local_to_map(position)
	#if !unit_locations.is_valid(tile):
		#unit_locations.erase_unit(tile)
	#else:
		#printerr("Came from no tile")
#
#func set_position_with_unit_locations() -> void:
	#var tile: Vector2i = unit_locations.local_to_map(position)
	#if unit_locations.is_valid(tile):
		#unit_locations.set_unit(tile)
	#else:
		#printerr("Two units on same tile")
