#extends Sprite2D
#var location: Vector2i
#var stops: Array = []
#var stop_number: int = -1
#var vertex_order: Array = []
#var route: Array = []
#
#var stop_information: Dictionary = {} #Maps location -> station, nothing, depot, ect
#var train_car
#
#var near_stop: bool = false
#var acceleration_direction: Vector2
#var velocity: Vector2
#var cargo_hold: train_hold = train_hold.new(location)
#var loading: bool = false
#var unloading: bool = false
#var ticker: float = 0
#var player_owner: int
#var stopped = true
#
#const acceptable_angles = [0, 60, 120, 180, 240, 300, 360]
#const MIN_SPEED = 20
#const MAX_SPEED = 100
#const ACCELERATION_SPEED = 20
#const BREAKING_SPEED = 40
#const TRAIN_CAR_SIZE = 16
#const LOAD_SPEED = 1
#const LOAD_TICK_AMOUNT = 1
#
#const train_car_scene = preload("res://Cargo/Cargo_Objects/train_car.tscn")
#
#@onready var map: TileMapLayer = get_parent()
#@onready var window: Window = $Window
#var cargo_controller
## Called when the node enters the scene tree for the first time.
#func _ready():
	#window.hide()
#
## Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
	#if !stopped:
		#position.x += velocity.x * delta
		#position.y += velocity.y * delta
		#update_train.rpc(position)
		#var rotation_basis = Vector2(0, 1)
		#if velocity.length() != 0:
			#var angle_degree = rad_to_deg(rotation_basis.angle_to(velocity))
			#update_train_rotation.rpc(round(angle_degree))
		#if near_stop:
			#deaccelerate_train(delta)
		#else:
			#accelerate_train(delta)
		#checkpoint_reached()
	#
	#ticker += delta
	#interact_stations()
	#check_ticker()
#
#
#@rpc("authority", "unreliable", "call_local")
#func update_train(new_position):
	#position = new_position
#
#@rpc("authority", "unreliable", "call_local")
#func update_train_rotation(new_rotation):
	#rotation_degrees = new_rotation
#
#func checkpoint_reached():
	#var route_local_pos = map.map_to_local(route[0])
	#check_near_next_stop()
	#
	##Reached the next tile
	#if position.distance_to(route_local_pos) < 10:
		#location = route.pop_front()
		#cargo_hold.update_location(location)
		#
		##Reached next vertex
		#if route.is_empty():
			#print("Near Vertex")
			#route = pathfind_to_next_vertex()
			#route.pop_front()
#
		##If route is still empty then stop reached
		#if route.is_empty():
			#print("Near stop")
			#increment_stop()
			#if decide_stop_action():
				#stop_train()
				#return
			#start_train()
		#else:
			#drive_train_to_route()
#
#func drive_train_to_route():
	#var direction = Vector2(map.map_to_local(route[0]) - map.map_to_local(location)).normalized()
	#acceleration_direction = direction
	#train_car.update_desination(position)
#
#func check_near_next_stop():
	#var next_stop_local_pos = map.map_to_local(stops[stop_number])
	#if position.distance_to(next_stop_local_pos) < (velocity.length() / BREAKING_SPEED * 60):
		#near_stop = true
#
#func increment_stop():
	#stop_number = (stop_number + 1) % stops.size()
#
#func decide_stop_action() -> bool:
	##Just a regular track
	#if !stops.has(location) or stop_information[location] == 3:
		#return false
	#var terminal_or_depot = map.get_depot_or_terminal(location)
	##A Depot
	#if stop_information[location] == 1:
		#terminal_or_depot.add_train(self)
		#visible = false
		#return true
	##Some sort of hold
	#elif stop_information[location] == 2:
		#unloading = true
		#return true
	#return false
#
#func get_speed() -> float:
	#return velocity.length()
#
#func accelerate_train(delta):
	#var speed = velocity.length()
	#speed = move_toward(speed, MAX_SPEED, delta * ACCELERATION_SPEED)
	#velocity = acceleration_direction * speed
#
#func deaccelerate_train(delta):
	#var speed = velocity.length()
	#speed = move_toward(speed, MIN_SPEED, delta * BREAKING_SPEED)
	#velocity = acceleration_direction * speed
#
#func pathfind_to_next_stop():
	#var possible = get_possible_vertices()
	#var end = map.get_vertex(stops[stop_number])
	#if possible.has(end):
		#vertex_order = [location, end.get_coordinates()]
	#else:
		#vertex_order = dijikstra_get_vertice_order(possible, end)
#
#func pathfind_to_next_vertex() -> Array:
	#if vertex_order.size() < 2:
		#return []
	#return create_route_to_next_vertex(vertex_order.pop_front(), vertex_order[0])
#
#func _input(event):
	#if event.is_action_pressed("click") and $Window/Routes/Add_Stop.button_pressed:
		#do_add_stop(map.get_cell_position())
	#elif event.is_action_pressed("click") and visible:
		#open_menu(map.get_mouse_local_to_map())
	#elif event.is_action_pressed("deselect"):
		#window.deselect_add_stop()
#
#@rpc("authority", "unreliable", "call_local")
#func create(new_location: Vector2i, new_cargo_controller, new_owner: int):
	#position = map.map_to_local(new_location)
	#location = new_location
	#cargo_controller = new_cargo_controller
	#player_owner = new_owner
	#prep_update_cargo_gui()
	#
	#train_car = train_car_scene.instantiate()
	#map.add_child(train_car)
	#train_car.update_train(self)
	#train_car.position = position
#
#func check_ticker():
	#if ticker > 1:
		#ticker = 0
#
#func did_mouse_click(mouse_pos: Vector2):
	#var match_x = mouse_pos.x > position.x - 48 and mouse_pos.x < position.x + 48
	#var match_y = mouse_pos.y > position.y - 48 and mouse_pos.y < position.y + 48
	#return match_x and match_y
#
#func open_menu(mouse_pos: Vector2):
	#if did_mouse_click(mouse_pos):
		#window.popup()
#
#func _on_window_close_requested():
	#window.deselect_add_stop()
	#window.hide()
#
#@rpc("any_peer", "unreliable", "call_local")
#func do_add_stop(new_location: Vector2i):
	#if map.is_tile_vertex(new_location):
		#add_stop.rpc(new_location)
#
#@rpc("authority", "unreliable", "call_local")
#func add_stop(new_location: Vector2i):
	#if stop_number == -1:
		#stop_number = 0
	#stops.append(new_location)
	#stop_information[new_location] = get_stop_info(new_location)
	#var routes: ItemList = $Window/Routes
	#routes.add_item(str(new_location))
	#
#
#func get_stop_info(location_to_check: Vector2i) -> int:
	#if map.is_location_depot(location_to_check):
		#return 1
	#elif map.is_location_hold(location_to_check):
		#return 2
	#return 3
#
#@rpc("any_peer", "unreliable", "call_local")
#func remove_stop(index: int):
	#stop_information.erase(stops[index])
	#stops.remove_at(index)
	#var routes: ItemList = $Window/Routes
	#routes.remove_item(index)
	#if stops.size() == 0:
		#stop_number = -1
		#stop_train()
		#return
	#elif stop_number == index:
		#increment_stop()
	#elif stop_number > index and stop_number != 0:
		#stop_number -= 1
#
#func clear_stops():
	#stops = []
	#stop_information.clear()
	#stop_number = -1
	#stopped = true
#
#func _on_start_pressed():
	#start_train()
#
#func _on_stop_pressed():
	#stop_train()
#
#func drive_train_from_depot():
	#pass
#
#@rpc("any_peer", "unreliable", "call_local")
#func start_train():
	#if stops.size() == 0:
		#return
	#near_stop = false
	#velocity = Vector2(0, 0)
	##pathfind_to_next_stop()
	#route = create_route_to_next_vertex(location, stops[0])
	##route = pathfind_to_next_vertex()
	#stopped = false
	#if route.is_empty() and stops.size() > 1:
		#increment_stop()
		#route = pathfind_to_next_vertex()
	#if route.is_empty():
		#stop_train()
		#return
	#drive_train_to_route()
#
#@rpc("any_peer", "unreliable", "call_local")
#func stop_train():
	#velocity = Vector2(0, 0)
	#acceleration_direction = Vector2(0, 0)
	#near_stop = false
	#stopped = true
#
#func interact_stations():
	#unload_train()
	#load_train()
#
#func load_train():
	#if loading and map.is_location_hold(location) and ticker > 1:
		#load_tick()
		#prep_update_cargo_gui()
		#if cargo_hold.is_full():
			#done_loading()
#
#func load_tick():
	#var amount_loaded = 0
	#var obj: terminal = map.get_depot_or_terminal(location)
	#var current_hold = obj.get_current_hold()
	#if hold_is_empty(current_hold):
		#done_loading()
	#for type in current_hold:
		#var amount = min(LOAD_TICK_AMOUNT - amount_loaded, current_hold[type])
		#var amount_actually_loaded = cargo_hold.add_cargo(type, amount)
		#amount_loaded += amount_actually_loaded
		#obj.remove_cargo(type, amount_actually_loaded)
		#if amount_loaded == LOAD_TICK_AMOUNT:
			#break
#
#func hold_is_empty(toCheck: Dictionary):
	#for value in toCheck.values():
		#if value != 0:
			#return false
	#return true
#
#func done_loading():
	#loading = false
	#start_train()
	#cargo_hold.finalize_cargo_collections()
#
#func unload_train():
	#var obj: terminal = map.get_depot_or_terminal(location)
	#if obj is station and unloading and ticker > 1:
		#unload_tick(obj)
		#prep_update_cargo_gui()
		#if cargo_hold.is_empty():
			#done_unloading()
		#
#
#func done_unloading():
	#unloading = false
	#loading = true
#
#func unload_tick(obj: station):
	#var amount_unloaded = 0
	#var accepts: Array = obj.get_accepts()
	#for type in accepts.size():
		#if accepts[type]:
			#var amount_desired = obj.get_desired_cargo(type)
			#var amount_to_transfer = min(amount_desired, LOAD_TICK_AMOUNT - amount_unloaded)
			#var cargo_array = cargo_hold.transfer_cargo(type, amount_to_transfer)
			#var cash = obj.deliver_cargo(cargo_array)
			#map.add_money_to_player(player_owner, cash)
			#amount_unloaded += cargo_array[1]
		#if amount_unloaded == LOAD_TICK_AMOUNT:
			#return
	#if amount_unloaded < LOAD_TICK_AMOUNT:
		#done_unloading()
#
#func prep_update_cargo_gui():
	#var cargo_array: Array[int] = cargo_hold.get_cargo_total_for_each_type()
	#var cargo_names: Array = cargo_controller.get_cargo_array()
	#update_cargo_gui.rpc(cargo_names, cargo_array)
#
#@rpc("authority", "unreliable", "call_local")
#func update_cargo_gui(names: Array, amounts: Array):
	#$Window/Goods.clear()
	#for type in names.size():
		#$Window/Goods.add_item(names[type] + ", " + str(amounts[type]))
#
#func add_train_car():
	#cargo_hold.change_max_storage(0, TRAIN_CAR_SIZE)
#
#func delete_train_car():
	#cargo_hold.change_max_storage(0, -TRAIN_CAR_SIZE)
#
#func dijikstra_get_vertice_order(vertices_directions: Dictionary, end: rail_vertex) -> Array:
	#var queue = priority_queue.new()
	#var tile_to_prev = {} # Vector2i -> Array[Tile for each direction]
	#var tile_out = {} # Vector2i -> Array[Tile for each direction]
	#var order = {} # Vector2i -> Array[indices in order for tile_to_prev, first one is the fastest]
	#var visited = {} # Vector2i -> Array[Bool for each direction]
	#visited[location] = get_train_dir_in_array()
	#var start = map.get_vertex(location)
	#for vertex: rail_vertex in vertices_directions:
		#if vertex != start:
			#intialize_tile_to_prev(tile_to_prev, vertex.get_coordinates(), (vertices_directions[vertex] + 3) % 6, location)
			#intialize_order(order, vertex.get_coordinates(), (vertices_directions[vertex] + 3) % 6)
		#intialize_visited(visited, vertex.get_coordinates(), vertices_directions[vertex])
		#if start != null:
			#intialize_tile_out(tile_out, start.get_coordinates(), get_direction(), vertex.get_coordinates())
		#queue.add_item(0, vertex)
	#
	#var loop = false
	#var found = false
	#var curr: rail_vertex
	#while !queue.is_empty():
		#curr = queue.pop_top()
		#for vertex: rail_vertex in curr.get_connections():
			#var direction = (curr.get_direction(vertex))
			#var recieving_direction = (vertex.get_direction(curr))
			#if vertex == curr:
				#recieving_direction = vertex.get_direction_self()
				#loop = true
			#if (visited[curr.get_coordinates()][direction] or visited[curr.get_coordinates()][(direction + 1) % 6] or visited[curr.get_coordinates()][(direction + 5) % 6]) and !check_visited(visited, vertex.get_coordinates(), (recieving_direction + 3) % 6):
				#intialize_visited(visited, vertex.get_coordinates(), (recieving_direction + 3) % 6)
				#intialize_tile_out(tile_out, curr.get_coordinates(), direction, vertex.get_coordinates())
				#queue.add_item(curr.get_length(vertex), vertex)
				#intialize_tile_to_prev(tile_to_prev, vertex.get_coordinates(), recieving_direction, curr.get_coordinates())
				#intialize_order(order, vertex.get_coordinates(), recieving_direction)
				#if vertex == end:
					#found = true
					#break
			#elif loop and (visited[curr.get_coordinates()][recieving_direction] or visited[curr.get_coordinates()][(recieving_direction + 1) % 6] or visited[curr.get_coordinates()][(recieving_direction + 5) % 6]) and !check_visited(visited, vertex.get_coordinates(), (direction + 3) % 6):
				#intialize_visited(visited, vertex.get_coordinates(), (direction + 3) % 6)
				#intialize_tile_out(tile_out, curr.get_coordinates(), recieving_direction, vertex.get_coordinates())
				#queue.add_item(curr.get_length(vertex), vertex)
				#intialize_tile_to_prev(tile_to_prev, vertex.get_coordinates(), direction, curr.get_coordinates())
				#intialize_order(order, vertex.get_coordinates(), direction)
				#loop = false
				#if vertex == end:
					#found = true
					#break
		#if found:
			#break
	#var route = [end.get_coordinates()]
	#var direction = null
	##print(tile_to_prev)
	##print("------")
	##print(tile_out)
	##print("------")
	##print(order)
	##print("------")
	##print(found)
	##return []
	#if found:
		#found = false
		#var curr_coords: Vector2i = end.get_coordinates()
		#while !found:
			#if direction != null:
				#for dir in order[curr_coords]:
					#if dir == direction or dir == (direction + 1) % 6 or dir == (direction + 5) % 6:
						#var old_coords = curr_coords
						#curr_coords = tile_to_prev[curr_coords][dir]
						#if curr_coords == location:
							#found = true
							#break
						#var temp = (tile_out[curr_coords]).find(old_coords)
						#if temp == -1:
							#print(curr_coords)
							#print("error")
							#return []
						#direction = (temp + 3) % 6
						#break
			#else:
				#var dir = order[curr_coords][0]
				#var old_coords = curr_coords
				#curr_coords = tile_to_prev[curr_coords][dir]
				#direction = ((tile_out[curr_coords]).find(old_coords) + 3) % 6
			#route.push_front(curr_coords)
			#if curr_coords == location:
				#found = true
	#if found:
		#return route
	#return []
		#
#
#func get_possible_vertices() -> Dictionary:
	#var toReturn = {}
	#if map.is_tile_vertex(location):
		#toReturn[map.get_vertex(location)] = get_direction()
		#return toReturn
	#var queue = [location]
	#var visited = {} # Vector2i -> Array[Bool for each direction]
	#visited[location] = get_train_dir_in_array()
	#var curr
	#while !queue.is_empty():
		#curr = queue.pop_front()
		#var cells_to_check = get_cells_in_front(curr, visited[curr])
		#for direction in cells_to_check.size():
			#var tile = cells_to_check[direction]
			#if tile != null and map.do_tiles_connect(curr, tile) and !check_visited(visited, tile, direction):
				#if map.is_tile_vertex(tile):
					#toReturn[map.get_vertex(tile)] = direction
				#else:
					#queue.append(tile)
					#intialize_visited(visited, tile, direction)
	#return toReturn
#
#func create_route_to_next_vertex(start: Vector2i, end: Vector2i) -> Array:
	#var queue = [start]
	#var tile_to_prev = {} # Vector2i -> Array[Tile for each direction]
	#var order = {} # Vector2i -> Array[indices in order for tile_to_prev, first one is the fastest]
	#var visited = {} # Vector2i -> Array[Bool for each direction]
	#visited[start] = get_train_dir_in_array()
	#var found = false
	#var curr: Vector2i
	#while !queue.is_empty():
		#curr = queue.pop_front()
		#var cells_to_check = get_cells_in_front(curr, visited[curr])
		#for direction in cells_to_check.size():
			#var tile = cells_to_check[direction]
			#if tile != null and map.do_tiles_connect(curr, tile) and !check_visited(visited, tile, direction):
				#intialize_visited(visited, tile, direction)
				#queue.push_back(tile)
				#intialize_tile_to_prev(tile_to_prev, tile, (direction + 3) % 6, curr)
				#intialize_order(order, tile, (direction + 3) % 6)
				#if tile == end:
					#found = true
					#break
		#if found:
			#break
	#var route = [end]
	#var direction = null
	#if found:
		#found = false
		#curr = end
		#while !found:
			#if direction != null:
				#for dir in order[curr]:
					#if can_direction_reach_dir(direction, dir) and tile_to_prev[curr][dir] != null:
						#curr = tile_to_prev[curr][dir]
						#direction = dir
			#else:
				#var dir = order[curr][0]
				#curr = tile_to_prev[curr][dir]
				#direction = dir
			#route.push_front(curr)
			#if curr == start:
				#found = true
	#if found:
		#return route
	#return []
#
#func can_direction_reach_dir(direction: int, dir: int) -> bool:
	#return dir == direction or dir == (direction + 1) % 6 or dir == (direction + 5) % 6
#
#func get_cells_in_front(coords: Vector2i, directions: Array) -> Array:
	#var index = 2
	#var toReturn = [null, null, null, null, null, null]
	#for cell in map.get_surrounding_cells(coords):
		#if directions[index] or directions[(index + 1) % 6] or directions[(index + 5) % 6]:
			#toReturn[index] = cell
		#index = (index + 1) % 6
	#return toReturn
#
#func check_visited(visited: Dictionary, coords: Vector2i, direction: int) -> bool:
	#if visited.has(coords):
		#return visited[coords][direction]
	#return false
#
#func intialize_visited(visited: Dictionary, coords: Vector2i, direction: int):
	#if !visited.has(coords):
		#visited[coords] = [false, false, false, false, false, false]
	#visited[coords][direction] = true
#
#func intialize_tile_to_prev(tile_to_prev: Dictionary, coords: Vector2i, direction: int, prev: Vector2i):
	#if !tile_to_prev.has(coords):
		#tile_to_prev[coords] = [null, null, null, null, null, null]
	#tile_to_prev[coords][direction] = prev
#
#func intialize_tile_out(tile_out: Dictionary, coords: Vector2i, direction: int, next: Vector2i):
	#if !tile_out.has(coords):
		#tile_out[coords] = [null, null, null, null, null, null]
	#tile_out[coords][direction] = next
#
#func intialize_order(order: Dictionary, coords: Vector2i, direction: int):
	#if !order.has(coords):
		#order[coords] = []
	#order[coords].append(direction)
#
#func get_train_dir_in_array() -> Array:
	#var toReturn = [false, false, false, false, false, false]
	#toReturn[get_direction()] = true
	#return toReturn
#
#func get_direction() -> int:
	#var dir: int = round(rad_to_deg(rotation))
	#if dir < 0:
		#dir = dir * -1 + 180
	#return find_closest_acceptable_angle(dir)
#
#func find_closest_acceptable_angle(input_angle: int) -> int:
	#var min_index = -1
	#var min_diff = 360
	#for index in acceptable_angles.size():
		#var diff = abs(acceptable_angles[index] - input_angle)
		#if diff < min_diff:
			#min_diff = diff
			#min_index = index
	#if min_index == 6:
		#min_index = 0
	#return min_index
#
#func get_neighbor_cell_given_direction(coords: Vector2i, num: int) -> Vector2i:
	#if num == 0:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_SIDE)
	#elif num == 1:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_RIGHT_SIDE)
	#elif num == 2:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_RIGHT_SIDE)
	#elif num == 3:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_SIDE)
	#elif num == 4:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_BOTTOM_LEFT_SIDE)
	#elif num == 5:
		#return map.get_neighbor_cell(coords, TileSet.CELL_NEIGHBOR_TOP_LEFT_SIDE)
	#return Vector2i(0, 0)
