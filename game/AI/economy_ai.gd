extends Node

#AI States
var id: int
var thread: Thread
var stored_tile: Vector2i
var pending_deferred_calls: int = 0

#Set of States
var world_map: TileMapLayer
var tile_ownership: TileMapLayer
var cargo_map: TileMapLayer = terminal_map.cargo_map
var cargo_values: Node = cargo_map.cargo_values
var rail_placer: Node

#Set of Actions
enum ai_actions {
	PLACE_FACTORY,
	CONNECT_FACTORY,
	CONNECT_STATION,
	CONNECT_TOWN
}

func _init(_id: int, _world_map: TileMapLayer) -> void: 
	thread = Thread.new()
	world_map = _world_map
	id = _id
	rail_placer = world_map.rail_placer
	tile_ownership = Utils.tile_ownership
	tile_ownership.add_player_to_country(id, Vector2i(96, -111))

func process() -> void:
	if thread.is_alive():
		return
	elif thread.is_started():
		thread.wait_to_finish()
	while pending_deferred_calls > 0:
		OS.delay_msec(100)
	thread.start(run_ai_cycle.bind())

func _exit_tree() -> void:
	thread.wait_to_finish()

func acknowledge_pending_deferred_call() -> void:
	pending_deferred_calls -= 1

func run_ai_cycle() -> void:
	var start: float = Time.get_ticks_msec()
	var action_type: ai_actions = choose_type_of_action()
	#Add more actions later
	if action_type == ai_actions.PLACE_FACTORY:
		place_factory(get_most_needed_cargo())
	elif action_type == ai_actions.CONNECT_FACTORY:
		connect_factory(stored_tile)
	elif action_type == ai_actions.CONNECT_STATION:
		connect_station(stored_tile)
	elif action_type == ai_actions.CONNECT_TOWN:
		connect_town(stored_tile)
	var end: float = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed for one cycle")

func choose_type_of_action() -> ai_actions:
	#Criteria to choose later
	#if are_there_unconnected_towns():
		#return ai_actions.CONNECT_TOWN
	#if are_there_unconnected_buildings():
		#return ai_actions.CONNECT_FACTORY
	#elif are_there_unconnected_stations():
		#return ai_actions.CONNECT_STATION
	return ai_actions.PLACE_FACTORY

func are_there_unconnected_towns() -> bool:
	var callable: Callable = Callable(terminal_map, "is_town")
	assert(callable.is_valid())
	return search_for(callable)

func are_there_unconnected_buildings() -> bool:
	var callable: Callable = Callable(terminal_map, "is_owned_construction_site")
	assert(callable.is_valid())
	return search_for(callable)

func search_for(target: Callable) -> bool:
	for tile: Vector2i in get_owned_tiles():
		if target.call(tile):
			var found: bool = false
			for cell: Vector2i in world_map.get_surrounding_cells(tile):
				if tile_ownership.is_owned(id, cell) and terminal_map.is_station(cell):
					found = true
			if !found:
				stored_tile = tile
				return true
	return false

func are_there_unconnected_stations() -> bool:
	for tile: Vector2i in get_owned_tiles():
		if terminal_map.is_station(tile):
			var found: bool = false
			for cell: Vector2i in world_map.get_surrounding_cells(tile):
				#Check if there is rail
				if world_map.do_tiles_connect(tile, cell):
					found = true
			if !found:
				stored_tile = tile
				return true
	return false

func place_factory(type: int) -> void:
	var best_tile: Vector2i
	if is_cargo_primary(type):
		best_tile = get_optimal_primary_industry(type)
		if best_tile != Vector2i(0, 0):
			create_factory(best_tile, type)

func create_factory(location: Vector2i, type: int) -> void:
	cargo_map.create_construction_site(id, location)
	for recipe_set: Array in recipe.get_set_recipes():
		for output: int in recipe_set[1]:
			if output == type:
				terminal_map.set_construction_site_recipe(location, recipe_set)
				break

func get_optimal_primary_industry(type: int) -> Vector2i:
	var best_location: Vector2i
	var score: int = -10000
	for tile: Vector2i in get_owned_tiles():
		if !Utils.is_tile_open(tile, id):
			continue
		#TODO: Will kill runtime if station isn't close or doesn't exist
		var distance: int = distance_to_closest_station(tile)
		var current_score: int = get_cargo_magnitude(tile, type) - round(distance / 7.0)
		if distance == 1:
			current_score += 5
		var free_tile: bool = false
		for cell: Vector2i in world_map.get_surrounding_cells(tile):
			if is_tile_connected_to_world(cell):
				free_tile = true
				break
		if !free_tile:
			continue
		if current_score > score:
			score = current_score
			best_location = tile
	return best_location

func is_tile_connected_to_world(coords: Vector2i) -> bool:
	var queue: Array = [coords]
	var visited: Dictionary = {}
	visited[coords] = 0
	
	while !queue.is_empty():
		var curr: Vector2i = queue.pop_front()
		for tile: Vector2i in world_map.get_surrounding_cells(curr):
			if !visited.has(tile):
				if terminal_map.is_town(tile) or terminal_map.is_station(tile):
					return true
				elif Utils.just_has_rails(tile, id):
					visited[tile] = 0
					queue.append(tile)
	return false

func get_cargo_magnitude(coords: Vector2i, type: int) -> int:
	return cargo_values.get_tile_magnitude(coords, type)

func distance_to_closest_station(coords: Vector2i) -> int:
	var target1: Callable = Callable(terminal_map, "is_station")
	var target2: Callable = Callable(terminal_map, "is_town")
	assert(target1.is_valid())
	assert(target2.is_valid())
	return info_of_closest_target(coords, target1, target2)[0]

func coords_of_closest_station(coords: Vector2i) -> Vector2i:
	var target1: Callable = Callable(terminal_map, "is_station")
	var target2: Callable = Callable(terminal_map, "is_town")
	assert(target1.is_valid())
	assert(target2.is_valid())
	return info_of_closest_target(coords, target1, target2)[1]

func info_of_closest_target(coords: Vector2i, target: Callable, secondary_target: Callable) -> Array:
	var queue: Array = [coords]
	var visited: Dictionary = {}
	visited[coords] = 0
	var closest_secondary_target: Array = []
	var cycles: int = 0
	while !queue.is_empty():
		var curr: Vector2i = queue.pop_front()
		cycles += 1
		if cycles > 1000 and closest_secondary_target != null:
			break
		for tile: Vector2i in world_map.get_surrounding_cells(curr):
			#TODO: Add logic to account for docks and disconnected territory
			#TODO: Add logic that the closest station isnt the best to connect to
			#TODO: Add logic about not checking tiles that have buildings but allow towns and rails
			if !visited.has(tile) and tile_ownership.is_owned(id, tile):
				if target.call(tile):
					return [(visited[curr] + 1), tile]
				elif tile_ownership.is_owned(id, tile):
					visited[tile] = visited[curr] + 1
					queue.append(tile)
				if closest_secondary_target == null and secondary_target.call(tile):
					closest_secondary_target = [visited[curr] + 1, tile]
	if closest_secondary_target.is_empty():
		return [10000000, null]
	return closest_secondary_target

func is_cargo_primary(type: int) -> bool:
	return terminal_map.amount_of_primary_goods > type

func get_most_needed_cargo() -> int:
	var cargo_fulfillment: Dictionary = get_town_fulfillment()
	var min_fulfilled: float = 100.0
	var type_to_return: int = -1
	for type: int in cargo_fulfillment:
		if cargo_fulfillment[type] < min_fulfilled:
			min_fulfilled = cargo_fulfillment[type]
			type_to_return = type
		#Grain
		elif type == 12 and cargo_fulfillment[type] * 0.8 < min_fulfilled:
			min_fulfilled = cargo_fulfillment[type]
			type_to_return = type
	return type_to_return

func get_town_fulfillment() -> Dictionary:
	var town_tiles: Array = get_town_tiles()
	var total: Dictionary = {}
	var towns: float = town_tiles.size()
	for town_tile: Vector2i in town_tiles:
		for type: int in terminal_map.get_town_wants(town_tile):
			if !total.has(type):
				total[type] = 0.0
			total[type] += terminal_map.get_town_fulfillment(town_tile, type)
	for type: int in total:
		total[type] = total[type] / towns
	return total

func connect_factory(coords: Vector2i) -> void:
	var closest_station: Vector2i = coords_of_closest_station(coords)
	place_station(coords, closest_station)

func place_station(center: Vector2i, dest: Vector2i) -> void:
	var info: Array = get_closest_info(center, dest)
	var orientation: int = info[1]
	var best: Vector2i = info[0]
	if best == Vector2i(0, 0):
		print("problems")
		return
	create_station(best, orientation)

func get_closest_info(center: Vector2i, dest: Vector2i) -> Array:
	#TODO: Add checking to see if orientation will be blocked
	var best: Vector2i
	var closest_dist: float = 100000.0
	var orientation: int
	var orientation_tracker: int = 2
	for tile: Vector2i in world_map.get_surrounding_cells(center):
		if Utils.is_tile_open(tile, id):
			var dist: float = tile.distance_to(dest)
			if closest_dist > dist:
				closest_dist = dist
				best = tile
				orientation = orientation_tracker
		orientation_tracker = (orientation_tracker + 1) % 6
	return [best, orientation]

func create_station(location: Vector2i, orientation: int) -> void:
	#Note: If needed on same process bad things will happen
	world_map.call_deferred_thread_group("set_cell_rail_placer_request", location, orientation, 2, id)
	pending_deferred_calls += 1

func connect_station(coords: Vector2i) -> void:
	var closest_station: Vector2i = coords_of_closest_station(coords)
	build_rail(coords, closest_station, Callable(Utils, "just_has_rails"))

func connect_town(coords: Vector2i) -> void:
	#Function is sort of a test and shouldn't ever be used
	assert(false)
	var target1: Callable = Callable(terminal_map, "is_town")
	#TODO: Should be dock but station as placeholder
	var target2: Callable = Callable(terminal_map, "is_station")
	assert(target1.is_valid())
	assert(target2.is_valid())
	var coords_of_closest_town: Vector2i = info_of_closest_target(coords, target1, target2)[1]
	#Choose closest orientation then add one / minus one to that for second station
	var info: Array = get_closest_info(coords, coords_of_closest_town)
	var orientation: int = info[1]
	var station1_coords: Vector2i = info[0]
	var station2_coords: Vector2i = get_closest_info(coords_of_closest_town, coords)[0]
	create_station(station1_coords, orientation)
	create_station(station2_coords, orientation)
	await world_map.get_tree().process_frame
	#Every await should be split into a different ai cycle to prevent having to use await or signals which dont work all the time
	var callable: Callable = Callable(Utils, "is_tile_open")
	build_rail(station1_coords, station2_coords, callable)
	await world_map.get_tree().process_frame
	var station3_coords: Vector2i = get_shared_tile_between(station1_coords, coords)[0]
	var station4_coords: Vector2i = get_shared_tile_between(station2_coords, coords_of_closest_town)[0]
	create_station(station3_coords, orientation)
	await world_map.get_tree().process_frame
	create_station(station4_coords, orientation)
	await world_map.get_tree().process_frame
	build_rail(station3_coords, station4_coords, callable)

func get_shared_tile_between(coords1: Vector2i, coords2: Vector2i) -> Array:
	var coords1_set: Dictionary = {}
	var shared: Array = []
	for tile: Vector2i in world_map.get_surrounding_cells(coords1):
		coords1_set[tile] = 1
	for tile: Vector2i in world_map.get_surrounding_cells(coords2):
		if coords1_set.has(tile):
			shared.append(tile)
	return shared

func get_owned_tiles() -> Array:
	return tile_ownership.get_owned_tiles(id)

func is_cell_available(coords: Vector2i) -> bool:
	return !terminal_map.is_tile_taken(coords) and tile_ownership.is_owned(id, coords) 

func get_town_tiles() -> Array:
	var toReturn: Array = []
	for tile: Vector2i in get_owned_tiles():
		if terminal_map.is_town(tile):
			toReturn.append(tile)
	return toReturn

func get_reward() -> float:
	return 0.0

func build_rail(start: Vector2i, end: Vector2i, criteria_for_tile: Callable) -> void:
	#3 Scenerios for building rails
	#1 Pt - to - pt -- Current
	#2 Build 2 Way's with signals in cities, pt - to - pt elsewhere
	#3 Connect to nearest rail and use signals.
	var route: Dictionary = get_rails_to_build(start, end, criteria_for_tile)
	for tile: Vector2i in route:
		for orientation: int in route[tile]:
			place_rail(tile, orientation)

func place_rail(coords: Vector2i, orientation: int) -> void:
	#Note: If needed on same process bad things will happen
	world_map.call_deferred_thread_group("set_cell_rail_placer_request", coords, orientation, 0, id)
	pending_deferred_calls += 1

func get_rails_to_build(from: Vector2i, to: Vector2i, criteria_for_tile: Callable) -> Dictionary:
	var starting_orientation: int = rail_placer.get_station_orientation(from)
	var ending_orientation: int = rail_placer.get_station_orientation(to)
	#TODO: Rail Algorithm navigates around pre-built rails, most solve
	var queue: Array = [from]
	var tile_to_prev: Dictionary = {} # Vector2i -> Array[Tile for each direction]
	var order: Dictionary = {} # Vector2i -> Array[indices in order for tile_to_prev, first one is the fastest]
	var visited: Dictionary = {} # Vector2i -> Array[Bool for each direction]
	visited[from] = [false, false, false, false, false, false]
	visited[from][swap_direction(starting_orientation)] = true
	visited[from][(starting_orientation)] = true
	
	#Used to only allow the top and bottom of the station to connect
	var check: Array = [null, null, null, null, null, null]
	var surrounding: Array[Vector2i] = world_map.get_surrounding_cells(from)
	check[(starting_orientation + 3) % 6] = (surrounding[(starting_orientation + 1) % 6])
	check[starting_orientation] = (surrounding[(starting_orientation + 4) % 6])
	
	var found: bool = false
	var curr: Vector2i
	while !queue.is_empty() and !found:
		curr = queue.pop_front()
		var cells_to_check: Array = get_cells_in_front(curr, visited[curr])
		if curr == from:
			cells_to_check = check
		for direction: int in cells_to_check.size():
			var cell: Vector2i = cells_to_check[direction]
			if cell == to and (direction == ending_orientation or direction == swap_direction(ending_orientation)):
				intialize_visited(visited, cell, direction)
				intialize_order(order, cell, direction)
				intialize_tile_to_prev(tile_to_prev, cell, direction, curr)
				found = true
				break
			elif cell != null and !check_visited(visited, cell, direction) and criteria_for_tile.call(cell, id):
				intialize_visited(visited, cell, direction)
				intialize_order(order, cell, direction)
				intialize_tile_to_prev(tile_to_prev, cell, direction, curr)
				queue.append(cell)
	
	var toReturn: Dictionary = {}
	var direction: int = -1
	var prev: Variant = null
	if found:
		var index: int = 0
		#To ensure it leads directly out of the station
		while direction != ending_orientation and direction != ((ending_orientation + 3) % 6):
			direction = order[to][index]
			index += 1
		curr = tile_to_prev[to][direction]
		found = false
		while !found:
			if prev != null:
				toReturn[prev].append((direction + 3) % 6)
			if curr == from and (can_direction_reach_dir(direction, swap_direction(starting_orientation)) or can_direction_reach_dir(direction, starting_orientation)):
				break
			toReturn[curr] = [direction]
			for dir: int in order[curr]:
				if can_direction_reach_dir(direction, dir) and tile_to_prev[curr][dir] != null:
					prev = curr
					curr = tile_to_prev[curr][dir]
					direction = dir
					break
	return toReturn

func can_direction_reach_dir(direction: int, dir: int) -> bool:
	return dir == direction or dir == (direction + 1) % 6 or dir == (direction + 5) % 6

func at_odd_angle(station_orientation: int, rail_orientation: int) -> bool:
	return rail_orientation == (station_orientation + 1) % 6 or rail_orientation == (station_orientation + 5) % 6

func swap_direction(num: int) -> int: 
	return (num + 3) % 6

func get_cells_in_front(coords: Vector2i, directions: Array) -> Array:
	var index: int = 2
	var toReturn: Array = [null, null, null, null, null, null]
	for cell: Vector2i in world_map.get_surrounding_cells(coords):
		if directions[index] or directions[(index + 1) % 6] or directions[(index + 5) % 6] and !terminal_map.is_tile_taken(cell):
			toReturn[index] = cell
		index = (index + 1) % 6
	return toReturn

func check_visited(visited: Dictionary, coords: Vector2i, direction: int) -> bool:
	if visited.has(coords):
		return visited[coords][direction]
	return false

func intialize_visited(visited: Dictionary, coords: Vector2i, direction: int) -> void:
	if !visited.has(coords):
		visited[coords] = [false, false, false, false, false, false]
	visited[coords][direction] = true

func intialize_tile_to_prev(tile_to_prev: Dictionary, coords: Vector2i, direction: int, prev: Vector2i) -> void:
	if !tile_to_prev.has(coords):
		tile_to_prev[coords] = [null, null, null, null, null, null]
	tile_to_prev[coords][direction] = prev

func intialize_order(order: Dictionary, coords: Vector2i, direction: int) -> void:
	if !order.has(coords):
		order[coords] = []
	order[coords].append(direction)
