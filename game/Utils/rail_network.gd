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
	var use_second_checker: bool = false
	#Multiple trains don't really work
	
	#Creates endnode connections
	while true:
		var endnode: rail_node = find_endnode()
		if endnode == null:
			#No unclaimed endnodes, rest will be largest unclaimed
			use_second_checker = true
			break
		service_node(endnode, curr_train_id)
		var other_coords: Vector2i = endnode.get_only_connected_node()
		var other_node: rail_node = network[other_coords]
		endnode.claim_best_connection(other_node, curr_train_id)
		service_node(other_node, curr_train_id)
		
		i = (i + 1) % number_of_trains
		curr_train_id = train_members[i]
		if i == 0:
			break
	
	#Fills in rest around the biggest stations
	while use_second_checker:
		var node: rail_node = get_biggest_node()
		service_node(node, curr_train_id)
		var other_node: rail_node = node.get_biggest_node()
		node.claim_best_connection(other_node, curr_train_id)
		service_node(other_node, curr_train_id)
		i = (i + 1) % number_of_trains
		curr_train_id = train_members[i]
		if i == 0:
			break
	#No ai_train will have more than 1 node by this point
	#Will run til all nodes completed
	while true:
		i = get_smallest_index()
		curr_train_id = train_members[i]
		var edge: rail_edge = get_best_edge_in_network(curr_train_id)
		if edge == null:
			#Should never happen as it means we have stations to serve but no edge to get there
			break
		edge.claim_edge(curr_train_id)
		service_node(edge.node1, curr_train_id)
		service_node(edge.node2, curr_train_id)
		if check_for_completion():
			break
	assign_train_routes()
	start_trains()

#TODO: Inefficient and doesn't real look very far
func find_endnode() -> rail_node:
	for node: rail_node in network.values():
		if node.connections.size() == 1 and node.weight > 0 and !node.is_serviced():
			return node
	#No endnodes
	return null

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
	var smallest: float = weight_serviced_array[0]
	var index: int = 0
	for i: int in range(1, weight_serviced_array.size()):
		if smallest > weight_serviced_array[i]:
			index = i
			smallest = weight_serviced_array[i]
	return index
	
func get_best_edge_in_network(train_id: int) -> rail_edge:
	#THis will return edges to empty nodes which serves no one
	var best_edge: rail_edge = null
	for node: rail_node in network.values():
		#Only include nodes that are serviced by train
		if !node.does_service(train_id):
			continue
		var edge: rail_edge = node.get_best_edge(train_id)
		if edge != null and (best_edge == null or edge.weight > best_edge.weight) and does_section_have_weighted_node(node, edge, train_id):
			best_edge = edge
	return best_edge

func does_section_have_weighted_node(source_node: rail_node, starting_edge: rail_edge, train_id: int) -> bool:
	var visited: Dictionary[rail_node, bool] = {}
	visited[source_node] = true
	var stack: Array[rail_edge] = [starting_edge]
	
	while !stack.is_empty():
		var current_edge: rail_edge = stack.pop_front()
		var node1: rail_node = current_edge.node1
		var node2: rail_node = current_edge.node2
		
		#Don't include visited nodes or nodes already serviced by another branch
		var is_node_available: Callable = func(node: rail_node, id: int) -> bool:
			return visited.has(node) or node.does_service(id)
		
		#Determines which side of edge we are on
		if is_node_available.call(node1, train_id) and is_node_available.call(node2, train_id):
			continue
		var dest: rail_node
		if is_node_available.call(node1, train_id):
			dest = node2
		else:
			dest = node1

		visited[dest] = true
		
		#Check if it has weight
		if dest.weight > 0:
			return true
		
		var in_dir: int = (current_edge as rail_edge).get_direction_to_node(dest)
		
		#DO checks on all edges out of dest that work for dest in
		for edge: rail_edge in dest.get_best_connections():
			if edge.is_traversable(in_dir, dest):
				stack.push_front(edge)
	
	return false

func check_for_completion() -> bool:
	for node: rail_node in network.values():
		#Only count stations
		if node.get_weight() > 0 and !node.is_serviced():
			return false
	return true

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
			var other_dir: int = (edge.get_direction_to_node(other_node) + 3) % 6
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
