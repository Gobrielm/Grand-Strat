class_name rail_network extends Node

var network_id: int
var network: Dictionary[Vector2i, rail_node]
var weight_serviced: Dictionary[int, float] #Train id -> weight serviced
var train_members: Array[int] #Train id
var all_network_coords: Dictionary[Vector2i, bool] #Set used for checking if network changed

func _init(p_network_id: int) -> void:
	network_id = p_network_id
	network = {}
	weight_serviced = {}
	train_members = []

func has_changed() -> bool:
	var world_map: TileMapLayer = Utils.world_map
	var tile_ownership: TileMapLayer = Utils.tile_ownership
	for coords: Vector2i in all_network_coords:
		for tile: Vector2i in world_map.get_surrounding_cells(coords):
			var id1: int = tile_ownership.get_player_id_from_cell(coords)
			var id2: int = tile_ownership.get_player_id_from_cell(tile)
			#If a tile in the network connects to a tile out of network then it has changed and both are in same country
			if world_map.do_tiles_connect(coords, tile) and !all_network_coords.has(tile) and id1 == id2:
				return true
	return false

func has_altered(coords: Vector2i) -> bool:
	#Coords is a new rail
	#If placed on existing network, new node potentially
	if all_network_coords.has(coords):
		return true
	var world_map: TileMapLayer = Utils.world_map
	var tile_ownership: TileMapLayer = Utils.tile_ownership
	for tile: Vector2i in all_network_coords:
		var id1: int = tile_ownership.get_player_id_from_cell(coords)
		var id2: int = tile_ownership.get_player_id_from_cell(tile)
		#New bordering rail
		if all_network_coords.has(tile) and world_map.do_tiles_connect(coords, tile) and id1 == id2:
			return true
	return false

func is_built() -> bool:
	return !network.is_empty()

func clear_network() -> void:
	network.clear()
	for id: int in train_members:
		weight_serviced[id] = 0.0

func has_coords(coords: Vector2i) -> bool:
	return path_find_to_node(coords)[0]

func path_find_to_node(start: Vector2i) -> Array:
	var map: TileMapLayer = Utils.world_map
	var queue: Array = [start]
	var visited: Dictionary = {} # Vector2i -> Array[Bool for each direction]
	visited[start] = Utils.rail_placer.get_track_connections(start)
	var curr: Vector2i
	while !queue.is_empty():
		curr = queue.pop_front()
		var cells_to_check: Array = get_cells_in_front(curr, visited[curr], map)
		for direction: int in cells_to_check.size():
			#Tile isn't a rail
			if cells_to_check[direction] == null:
				continue
			var tile: Vector2i = cells_to_check[direction]
			#Tiles do not connect by rail or has visited
			if !map.do_tiles_connect(curr, tile) or has_visited(visited, tile, direction):
				continue
			intialize_visited(visited, tile, direction)
			queue.push_back(tile)
			if is_node(tile):
				return [true, tile]
	return [false, start]

func get_number_of_trains() -> int:
	return train_members.size()

func add_train_to_network(train_id: int) -> void:
	train_members.append(train_id)
	weight_serviced[train_id] = 0.0

func is_train_memeber(train_id: int) -> bool:
	return train_members.has(train_id)

func get_member_from_network() -> int:
	return train_members.pick_random()

func network_has_node(node_coords: Vector2i) -> bool:
	return network.has(node_coords)

class rail_info:
	func _init(p_source: Vector2i, p_dist: int, p_output_dir: int) -> void:
		source = p_source
		dist = p_dist
		output_dir = p_output_dir
	
	var source: Vector2i
	var dist: int
	var output_dir: int
	
	func copy() -> rail_info:
		return rail_info.new(source, dist, output_dir)

func create_network(start: Vector2i) -> void:
	#Assuming start is a node
	if !is_node(start):
		var results: Array = path_find_to_node(start)
		if !results[0]:
			#No nodes attached to start
			return
		start = results[1]
		
	clear_network()
	
	var map: TileMapLayer = Utils.world_map
	var stack: Array = [start]
	var visited: Dictionary = {} # Vector2i -> Array[Bool for each direction]
	var dist: Dictionary[Vector2i, rail_info] = {} #Array[Vector2i(Source), int(dist)]
	dist[start] = rail_info.new(start, 0, -1)
	create_node(start)
	all_network_coords[start] = true
	visited[start] = Utils.rail_placer.get_track_connections(start)
	var curr: Vector2i
	while !stack.is_empty():
		curr = stack.pop_front()
		#TODO: Clear later, For debugging
		#map.highlight_cell(curr)
		#await map.get_tree().create_timer(0.5).timeout
		var cells_to_check: Array = get_cells_in_front(curr, visited[curr], map)
		for direction: int in cells_to_check.size():
			#Tile isn't a rail or has visitd
			if cells_to_check[direction] == null or has_visited(visited, cells_to_check[direction], direction):
				continue
			var tile: Vector2i = cells_to_check[direction]
			#Tiles do not connect by rail
			if !map.do_tiles_connect(curr, tile):
				continue
			intialize_visited(visited, tile, direction)
			stack.push_front(tile)
			all_network_coords[tile] = true
			#If already traversed this then skip
			if dist.has(tile):
				#If new edge, but existing vertex then add
				if dist[curr].source != tile and is_node(tile):
					#Nodes next to each other, don't double connect
					if is_node(curr):
						continue
					connect_nodes(tile, dist[curr].source, dist[curr].dist + 1, (direction + 3) % 6, dist[curr].output_dir)
				#Creating when checking node so okay to replace and prevent nodes from doing this
				elif dist[tile].dist == 1 and dist[curr].source != dist[tile].source and dist[curr].dist != 0:
					dist[tile] = dist[curr].copy()
					dist[tile].dist += 1
				continue
			dist[tile] = dist[curr].copy()
			dist[tile].dist += 1
			if dist[tile].dist == 1:
				dist[tile].output_dir = direction
			if is_node(tile):
				create_node(tile)
				connect_nodes(tile, dist[tile].source, dist[tile].dist, (direction + 3) % 6, dist[tile].output_dir)
				dist[tile] = rail_info.new(tile, 0, -1)

func is_node(tile: Vector2i) -> bool:
	return terminal_map.is_station(tile) or map_data.get_instance().is_depot(tile) or Utils.rail_placer.get_track_connection_count(tile) >= 3

func get_cells_in_front(coords: Vector2i, directions: Array, map: TileMapLayer) -> Array:
	var index: int = 2
	var toReturn: Array = [null, null, null, null, null, null]
	for cell: Vector2i in map.get_surrounding_cells(coords):
		if directions[index] or directions[(index + 1) % 6] or directions[(index + 5) % 6]:
			toReturn[index] = cell
		index = (index + 1) % 6
	return toReturn

func intialize_visited(visited: Dictionary, coords: Vector2i, direction: int) -> void:
	if !visited.has(coords):
		visited[coords] = [false, false, false, false, false, false]
	visited[coords][direction] = true

func fill_visited(visited: Dictionary, coords: Vector2i) -> void:
	visited[coords] = [true, true, true, true, true, true]

func intialize_both_sides_visited(visited: Dictionary, coords: Vector2i) -> void:
	for dir: int in range(6):
		if visited[coords][dir]:
			visited[coords][(dir + 3) % 6] = true

func has_visited(visited: Dictionary, coords: Vector2i, direction: int) -> bool:
	return visited.has(coords) and visited[coords][direction]

func has_visited_with_turning(visited: Dictionary, coords: Vector2i, direction: int) -> bool:
	return visited.has(coords) and (visited[coords][direction] or visited[coords][(direction + 1) % 6] or visited[coords][(direction + 5) % 6])

func create_node(coords: Vector2i) -> void:
	if !network.has(coords):
		var weight: int = 0
		var stat: station = terminal_map.get_station(coords)
		if stat != null:
			weight = stat.get_orders_magnitude()
			weight += 1
		network[coords] = rail_node.new(coords, weight)

func connect_nodes(coords1: Vector2i, coords2: Vector2i, dist: int, output_dir1: int, output_dir2: int) -> void:
	if network.has(coords1) and network.has(coords2):
		var node1: rail_node = network[coords1]
		var node2: rail_node = network[coords2]
		#TODO: Add directions for edges
		var edge: rail_edge = rail_edge.new(node1, node2, 10.0 / dist, output_dir1, output_dir2)
		node1.connect_nodes(edge)
		node2.connect_nodes(edge)

func rebuild_network(_effected_tiles: Array[Vector2i]) -> void:
	#Works with existing network to rebuild altered parts
	pass

func get_rail_node(coords: Vector2i) -> rail_node:
	return network[coords]

func split_up_network_among_trains() -> void:
	clear_ownership()
	#Network cannot be completed
	if network.size() <= 1:
		return
	
	var number_of_trains: int = get_number_of_trains()
	var i: int = 0
	var curr_train_id: int = train_members[i]
	#Multiple trains don't really work
	
	var train_starting_location: Dictionary[int, rail_node] = establish_first_sets_of_routes(number_of_trains)
	
	while true:
		i = get_smallest_index()
		curr_train_id = train_members[i]
		#Using starting location to ensure subnetwork is tightnit
		if !connect_to_simplest_station(train_starting_location[curr_train_id], curr_train_id):
			#Failed, isolated somehow?
			assert(false)
		if check_for_completion():
			break
	
	var trains_that_need_to_overlap: Array[int] = check_for_overlap()
	if trains_that_need_to_overlap.size() != 0:
		for id: int in trains_that_need_to_overlap:
			#Do some connecting
			add_overlap(train_starting_location[curr_train_id], id)
	
	assign_train_routes()
	start_trains()

#Will give every train at least a connection between two stations, returns each trains start
func establish_first_sets_of_routes(number_of_trains: int) -> Dictionary[int, rail_node]:
	var toReturn: Dictionary[int, rail_node] = {}
	var use_second_checker: bool = false
	var i: int = 0
	var curr_train_id: int = train_members[i]
	while true:
		var endnode: rail_node = find_station_endnode()
		if endnode == null:
			#No unclaimed endnodes, rest will be largest unclaimed
			use_second_checker = true
			break
		toReturn[curr_train_id] = endnode
		service_node(endnode, curr_train_id)
		if !connect_to_simplest_station(endnode, curr_train_id):
			#Failed, isolated somehow?
			assert(false)
		
		i = (i + 1) % number_of_trains
		curr_train_id = train_members[i]
		if i == 0:
			break
	
	#Fills in rest around the biggest stations
	while use_second_checker:
		
		var node: rail_node = get_biggest_node()
		service_node(node, curr_train_id)
		
		toReturn[curr_train_id] = node
		
		if !connect_to_simplest_station(node, curr_train_id):
			#Failed, isolated somehow?
			assert(false)
		
		i = (i + 1) % number_of_trains
		curr_train_id = train_members[i]
		if i == 0:
			break
	return toReturn

#TODO: Inefficient and doesn't real look very far
func find_station_endnode() -> rail_node:
	for node: rail_node in network.values():
		if node.connections.size() == 1 and node.weight > 0 and !node.is_serviced():
			return node
	#No endnodes
	return null

func connect_to_simplest_station(start: rail_node, train_id: int, callable: Callable = Callable()) -> bool:
	var queue: Array[rail_node] = [start]
	var back_to_start: Dictionary[rail_node, Array] = {}
	var dist: Dictionary[rail_node, int] = {} #Int is distance in edges, not actual distance
	dist[start] = 0
	back_to_start[start] = []
	#might be issues with overriding visited but we'll see
	var visited: Dictionary[rail_node, int] = {} #Int is direction
	visited[start] = -1
	
	var dest: rail_node = null
	
	while !queue.is_empty():
		var current: rail_node = (queue.pop_back() as rail_node)
		#Checks to make sure that unclaimed stations are priorized, but still goes for close stations
		if callable.is_null() and current.weight > 0 and !current.does_service(train_id):
			if dest != null and !current.is_serviced():
				if dist[current] * 0.5 < dist[dest]:
					dest = current
				break
			elif dest == null:
				dest = current
				if !current.is_serviced():
					break
		elif !callable.is_null():
			if callable.call(current, train_id):
				dest = current
				break
			
		var input_dir: int = visited[current]
		for edge: rail_edge in current.get_best_connections(input_dir):
			var node: rail_node = edge.get_other_node(current)
			var in_dir: int = edge.get_in_dir_to_node(node)
			dist[node] = dist[current] + 1
			queue.push_back(node)
			back_to_start[node] = back_to_start[current].duplicate()
			back_to_start[node].append(edge)
			visited[node] = in_dir
	
	if dest == null:
		return false
	
	for edge: rail_edge in back_to_start[dest]:
		edge.claim_edge(train_id)
		service_node(edge.node1, train_id)
		service_node(edge.node2, train_id)
	
	return true

func add_overlap(start: rail_node, train_id: int) -> void:
	#Surveys for closest station owned by other node
	if !connect_to_simplest_station(start, train_id, foo):
		#AAAAA
		assert(false)
	pass

func foo(current_node: rail_node, train_id: int) -> bool:
	var size: int = current_node.serviced_by.size()
	if size >= 2:
		return true
	elif size >= 1 and !current_node.does_service(train_id):
		return true
	return false

#Safe to call on already serviced nodes
func service_node(node: rail_node, train_id: int) -> void:
	if node.does_service(train_id):
		return
	var old_weight: float = node.get_weight()
	node.service_node(train_id)
	var new_weight: float = node.get_weight()
	for other_id: int in node.serviced_by:
		weight_serviced[other_id] -= old_weight
		weight_serviced[other_id] += new_weight
	weight_serviced[train_id] += new_weight
	
func get_biggest_node() -> rail_node:
	var biggest: rail_node = null
	for node: rail_node in network.values():
		#Divides by serviced by to make big nodes get serviced by multiple if big, and not if small
		if node != null and (biggest == null and node.weight > 0) or (biggest != null and node.weight / (node.serviced_by.size() + 1) > biggest.weight):
			biggest = node
	if biggest == null:
		return network.values().pick_random()
	return biggest

func get_smallest_index() -> int:
	var weight_serviced_array: Array = weight_serviced.values()
	var index: int = randi_range(0, weight_serviced_array.size()) #Pick random for testing mostly
	var smallest: float = weight_serviced_array[index]
	for i: int in range(1, weight_serviced_array.size()):
		if smallest > weight_serviced_array[i]:
			index = i
			smallest = weight_serviced_array[i]
	return index

func check_for_completion() -> bool:
	for node: rail_node in network.values():
		#Only count stations
		if node.get_weight() == 0:
			continue
		
		if !node.is_serviced():
			return false
	return true

#Returns train_ids that need to overlap
func check_for_overlap() -> Array[int]:
	var trains_overlap: Dictionary[int, bool] = {}
	for node: rail_node in network.values():
		#Only count stations
		if node.get_weight() == 0:
			continue
		#If multiple trains service this station then they have overlap
		if node.serviced_by.size() > 1:
			for id: int in node.serviced_by:
				trains_overlap[id] = true
	
	var toReturn: Array[int] = []
	for id: int in train_members:
		if !trains_overlap.has(id):
			toReturn.append(id)
	return toReturn

func clear_ownership() -> void:
	for id: int in train_members:
		weight_serviced[id] = 0.0
	for node: rail_node in network.values():
		node.clear_ownership()

func assign_train_routes() -> void:
	var train_manager_obj: train_manager = train_manager.get_instance()
	for train_id: int in train_members:
		var ai_train_obj: ai_train = train_manager_obj.get_ai_train(train_id)
		ai_train_obj.remove_all_stops()
		assign_train_route(ai_train_obj)
		
func assign_train_route(ai_train_obj: ai_train) -> void:
	var id: int = ai_train_obj.id
	var next: rail_node = find_owned_endnode(id)
	var in_dir: int = -1
	var visited: Dictionary[Vector2i, Array] = {}

	while next != null:
		var current_node: rail_node = next
		next = null
		ai_train_obj.add_stop(current_node.coords)
		if current_node.weight > 0:
			#Can leave in any direction from stations
			in_dir = -1
		
		for edge: rail_edge in current_node.get_owned_edges(id):
			var other_node: rail_node = edge.get_other_node(current_node)
			var other_dir: int = (edge.get_out_dir_from_node(other_node) + 3) % 6
			if has_visited(visited, other_node.coords, other_dir):
				continue
			#Can reach
			if (other_dir == in_dir or (other_dir + 1) % 6 == in_dir or ( other_dir + 5) % 6 == in_dir or in_dir == -1):
				if in_dir != -1:
					intialize_visited(visited, other_node.coords, in_dir)
				next = other_node
				in_dir = other_dir

func find_owned_endnode(train_id: int) -> rail_node:
	for node: rail_node in network.values():
		if node.does_service(train_id):
			if node.get_connects_to_owned_nodes(train_id) == 1 and node.weight > 0:
				return node
	#No endnodes, must be loop so return one of them
	return network.values()[0]

func start_trains() -> void:
	var train_manager_obj: train_manager = train_manager.get_instance()
	for train_id: int in train_members:
		train_manager_obj.get_ai_train(train_id).start_train()

func print_edges() -> void:
	var set_edges: Dictionary[rail_edge, bool] = {}
	for node: rail_node in network.values():
		for edge: rail_edge in node.get_edges():
			if !set_edges.has(edge):
				set_edges[edge] = true
				print(edge)

func _to_string() -> String:
	var toReturn: String = ""
	for node: rail_node in network.values():
		toReturn += str(node)
	return toReturn
