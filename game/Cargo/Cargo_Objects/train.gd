class_name train extends Sprite2D
var location: Vector2i
var stops: Array = []
var stop_number: int = -1
var route: Array = []

var train_car: Node
var id: int

var near_stop: bool = false
var acceleration_direction: Vector2
var velocity: Vector2
var cargo_hold: hold = hold.new(location, player_owner)
var loading: bool = false
var unloading: bool = false
var ticker: float = 0
var player_owner: int
var stopped: bool = true

const acceptable_angles: Array = [0, 60, 120, 180, 240, 300, 360]
const MIN_SPEED: int = 20
const MAX_SPEED: int = 100
const ACCELERATION_SPEED: int = 20
const BREAKING_SPEED: int = 40
const TRAIN_CAR_SIZE: int = 16
const LOAD_SPEED: int = 1
const LOAD_TICK_AMOUNT: int = 5

const train_car_scene: PackedScene = preload("res://Cargo/Cargo_Objects/train_car.tscn")
var map: TileMapLayer

@onready var window: Window = $Train_Window

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	window.hide()

@rpc("authority", "unreliable", "call_local")
func create(new_location: Vector2i, new_owner: int, p_id: int) -> void:
	map = Utils.world_map
	position = map.map_to_local(new_location)
	location = new_location
	player_owner = new_owner
	id = p_id
	prep_update_cargo_gui()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	if !stopped:
		position.x += velocity.x * delta
		position.y += velocity.y * delta
		update_train.rpc(position)
		var rotation_basis: Vector2 = Vector2(0, 1)
		if velocity.length() != 0:
			var angle_degree: int = round(rad_to_deg(rotation_basis.angle_to(velocity)))
			update_train_rotation.rpc(round(angle_degree))
		if near_stop:
			deaccelerate_train(delta)
		else:
			accelerate_train(delta)
		checkpoint_reached()
	
	ticker += delta
	interact_stations()
	check_ticker()

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("click") and is_selecting_route():
		do_add_stop(map.get_cell_position())
	elif event.is_action_pressed("click") and visible:
		open_menu(map.get_mouse_local_to_map())
	elif event.is_action_pressed("deselect"):
		window.deselect_add_stop()

@rpc("authority", "unreliable", "call_local")
func update_train(p_position: Vector2i) -> void:
	position = p_position

@rpc("authority", "unreliable", "call_local")
func update_train_rotation(new_rotation: int) -> void:
	rotation_degrees = new_rotation

func checkpoint_reached() -> void:
	var route_local_pos: Vector2 = map.map_to_local(route[0])
	check_near_next_stop()
	
	#Reached the next tile
	if position.distance_to(route_local_pos) < 10:
		location = route.pop_front()
		cargo_hold.update_location(location)
		
		#If route is still empty then stop reached
		if route.is_empty():
			increment_stop()
			if decide_stop_action():
				stop_train()
				return
			start_train()
		else:
			drive_train_to_route()

func drive_train_to_route() -> void:
	var direction: Vector2 = Vector2(map.map_to_local(route[0]) - position).normalized()
	#map.map_to_local(location)
	acceleration_direction = direction

func check_near_next_stop() -> void:
	var next_stop_local_pos: Vector2i = map.map_to_local(stops[stop_number])
	if position.distance_to(next_stop_local_pos) < (velocity.length() / BREAKING_SPEED * 60):
		near_stop = true

func increment_stop() -> void:
	stop_number = (stop_number + 1) % stops.size()

func decide_stop_action() -> bool:
	var map_data_obj: map_data = map_data.get_instance()
	
	if map_data_obj.is_depot(location):
		var depot: Node = map_data_obj.get_depot(location)
		depot.add_train(self)
		go_into_depot.rpc()
		return true
	elif terminal_map.is_station(location):
		unloading = true
		return true
	return false

@rpc("authority", "unreliable", "call_local")
func go_into_depot() -> void:
	visible = false

@rpc("authority", "unreliable", "call_local")
func go_out_of_depot(new_dir: int) -> void:
	visible = true
	rotation = new_dir * 60
	start_train()

func get_speed() -> float:
	return velocity.length()

func accelerate_train(delta: float) -> void:
	var speed: float = velocity.length()
	speed = move_toward(speed, MAX_SPEED, delta * ACCELERATION_SPEED)
	velocity = acceleration_direction * speed

func deaccelerate_train(delta: float) -> void:
	var speed: float = velocity.length()
	speed = move_toward(speed, MIN_SPEED, delta * BREAKING_SPEED)
	velocity = acceleration_direction * speed

func is_selecting_route() -> bool:
	return state_machine.is_selecting_route()

func check_ticker() -> void:
	if ticker > 1:
		ticker = 0

func did_mouse_click(mouse_pos: Vector2) -> bool:
	var match_x: bool = mouse_pos.x > position.x - 48 and mouse_pos.x < position.x + 48
	var match_y: bool = mouse_pos.y > position.y - 48 and mouse_pos.y < position.y + 48
	return match_x and match_y

func open_menu(mouse_pos: Vector2) -> void:
	if did_mouse_click(mouse_pos) and map.is_controlling_camera():
		window.popup()

func _on_window_close_requested() -> void:
	window.deselect_add_stop()
	window.hide()

@rpc("any_peer", "unreliable", "call_local")
func do_add_stop(new_location: Vector2i) -> void:
	if map.is_location_valid_stop(new_location):
		add_stop.rpc(new_location)

@rpc("authority", "unreliable", "call_local")
func add_stop(new_location: Vector2i) -> void:
	if stop_number == -1:
		stop_number = 0
	stops.append(new_location)
	var routes: ItemList = $Train_Window/Routes
	routes.add_item(str(new_location))

#TODO: Stuff with stations, downstream

@rpc("any_peer", "unreliable", "call_local")
func remove_stop(index: int) -> void:
	stops.remove_at(index)
	var routes: ItemList = $Train_Window/Routes
	routes.remove_item(index)
	if stops.size() == 0:
		stop_number = -1
		stop_train()
		return
	elif stop_number == index:
		increment_stop()
	elif stop_number > index and stop_number != 0:
		stop_number -= 1

func clear_stops() -> void:
	stops = []
	stop_number = -1
	stopped = true

func _on_start_pressed() -> void:
	start_train()

func _on_stop_pressed() -> void:
	stop_train()

func drive_train_from_depot() -> void:
	#TODO
	pass

@rpc("any_peer", "unreliable", "call_local")
func start_train() -> void:
	if stops.size() == 0:
		return
	near_stop = false
	velocity = Vector2(0, 0)
	route = pathfind_to_next_stop()
	stopped = false
	if route.is_empty() and stops.size() > 1:
		increment_stop()
		route = pathfind_to_next_stop()
	if route.is_empty():
		stop_train()
		return
	drive_train_to_route()

@rpc("any_peer", "unreliable", "call_local")
func stop_train() -> void:
	velocity = Vector2(0, 0)
	acceleration_direction = Vector2(0, 0)
	near_stop = false
	stopped = true

func interact_stations() -> void:
	unload_train()
	load_train()

func load_train() -> void:
	if loading and terminal_map.is_hold(location) and ticker > 1:
		load_tick()
		prep_update_cargo_gui()
		if cargo_hold.is_full():
			done_loading()

func load_tick() -> void:
	var amount_loaded: int = 0
	var obj: station = terminal_map.get_station(location)
	var current_hold: Dictionary = obj.get_current_hold()
	if hold_is_empty(current_hold):
		done_loading()
	for type: int in current_hold:
		if !obj.does_accept(type):
			var amount: int = min(LOAD_TICK_AMOUNT - amount_loaded, current_hold[type])
			var amount_actually_loaded: int = cargo_hold.add_cargo(type, amount)
			amount_loaded += amount_actually_loaded
			obj.remove_cargo(type, amount_actually_loaded)
			if amount_loaded == LOAD_TICK_AMOUNT:
				break

func hold_is_empty(toCheck: Dictionary) -> bool:
	for value: int in toCheck.values():
		if value != 0:
			return false
	return true

func done_loading() -> void:
	loading = false
	start_train()

func unload_train() -> void:
	var obj: station = terminal_map.get_station(location)
	if obj != null and unloading and ticker > 1:
		unload_tick(obj)
		prep_update_cargo_gui()
		if cargo_hold.is_empty():
			done_unloading()

func done_unloading() -> void:
	unloading = false
	loading = true

func unload_tick(obj: station) -> void:
	var amount_unloaded: int = 0
	var accepts: Dictionary = obj.get_accepts()
	for type: int in accepts:
		var amount_desired: int = obj.get_desired_cargo(type)
		var amount_to_transfer: int = min(amount_desired, LOAD_TICK_AMOUNT - amount_unloaded)
		var amount: int = cargo_hold.transfer_cargo(type, amount_to_transfer)
		obj.add_cargo(type, amount)
		amount_unloaded += amount
		
		if amount_unloaded == LOAD_TICK_AMOUNT:
			return
	if amount_unloaded < LOAD_TICK_AMOUNT:
		done_unloading()

func prep_update_cargo_gui() -> void:
	var cargo_names: Array = terminal_map.get_cargo_array()
	var cargo_dict: Dictionary = cargo_hold.get_current_hold()
	update_cargo_gui.rpc(cargo_names, cargo_dict)

@rpc("authority", "unreliable", "call_local")
func update_cargo_gui(names: Array, amounts: Dictionary) -> void:
	$Train_Window/Goods.clear()
	for type: int in amounts:
		if amounts[type] != 0:
			$Train_Window/Goods.add_item(names[type] + ", " + str(amounts[type]))

func add_train_car() -> void:
	cargo_hold.change_max_storage(0, TRAIN_CAR_SIZE)

func delete_train_car() -> void:
	cargo_hold.change_max_storage(0, -TRAIN_CAR_SIZE)

func pathfind_to_next_stop() -> Array:
	return create_route_between_start_and_end(map.local_to_map(position), stops[stop_number])

func create_route_between_start_and_end(start: Vector2i, end: Vector2i) -> Array:
	var queue: Array = [start]
	var tile_to_prev: Dictionary = {} # Vector2i -> Array[Tile for each direction]
	var order: Dictionary = {} # Vector2i -> Array[indices in order for tile_to_prev, first one is the fastest]
	var visited: Dictionary = {} # Vector2i -> Array[Bool for each direction]
	visited[start] = get_train_dir_in_array()
	var found: bool = false
	var curr: Vector2i
	while !queue.is_empty():
		curr = queue.pop_front()
		var cells_to_check: Array = get_cells_in_front(curr, visited[curr])
		for direction: int in cells_to_check.size():
			var tile: Vector2i = cells_to_check[direction]
			if tile != null and map.do_tiles_connect(curr, tile) and !check_visited(visited, tile, direction):
				intialize_visited(visited, tile, direction)
				queue.push_back(tile)
				intialize_tile_to_prev(tile_to_prev, tile, swap_direction(direction), curr)
				intialize_order(order, tile, swap_direction(direction))
				if tile == end:
					found = true
					break
		if found:
			break
	if found:
		return get_route_from_visited(order, tile_to_prev, start, end)
	return []

func get_route_from_visited(order: Dictionary, tile_to_prev: Dictionary, start: Vector2i, end: Vector2i) -> Array:
	var to_return: Array = [end]
	var direction: int = order[end][0]
	var curr: Vector2i = tile_to_prev[end][direction]
	var found: bool = false
	while !found:
		to_return.push_front(curr)
		if curr == start and can_direction_reach_dir(swap_direction(direction), get_direction()):
			found = true
			to_return.pop_front()
			break
		for dir: int in order[curr]:
			if can_direction_reach_dir(direction, dir) and tile_to_prev[curr][dir] != null:
				curr = tile_to_prev[curr][dir]
				direction = dir
				#Possibly needs break
	if !found:
		return []
	return to_return

func can_direction_reach_dir(direction: int, dir: int) -> bool:
	return dir == direction or dir == (direction + 1) % 6 or dir == (direction + 5) % 6

func swap_direction(num: int) -> int: 
	return (num + 3) % 6

func get_cells_in_front(coords: Vector2i, directions: Array) -> Array:
	var index: int = 2
	var toReturn: Array = [null, null, null, null, null, null]
	for cell: Vector2i in map.get_surrounding_cells(coords):
		if directions[index] or directions[(index + 1) % 6] or directions[(index + 5) % 6]:
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

func get_train_dir_in_array() -> Array:
	var toReturn: Array = [false, false, false, false, false, false]
	toReturn[get_direction()] = true
	if terminal_map.is_station(location):
		toReturn[swap_direction(get_direction())] = true
	return toReturn

func get_direction() -> int:
	var dir: int = round(rad_to_deg(rotation))
	if dir < 0:
		dir = dir * -1 + 180
	dir += 180
	return find_closest_acceptable_angle(dir)

func find_closest_acceptable_angle(input_angle: int) -> int:
	var min_index: int = -1
	var min_diff: int = 360
	for index: int in acceptable_angles.size():
		var diff: int = abs(acceptable_angles[index] - input_angle)
		if diff < min_diff:
			min_diff = diff
			min_index = index
	if min_index == 6:
		min_index = 0
	return min_index

func get_neighbor_cell_given_direction(coords: Vector2i, num: int) -> Vector2i:
	if num == 0:
		return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_SIDE)
	elif num == 1:
		return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_RIGHT_SIDE)
	elif num == 2:
		return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)
	elif num == 3:
		return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_SIDE)
	elif num == 4:
		return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_LEFT_SIDE)
	elif num == 5:
		return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_LEFT_SIDE)
	return Vector2i(0, 0)
