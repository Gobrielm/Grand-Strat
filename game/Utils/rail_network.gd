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
	var tile_ownership_obj: tile_ownership = tile_ownership.get_instance()
	for coords: Vector2i in all_network_coords:
		for tile: Vector2i in world_map.get_surrounding_cells(coords):
			var id1: int = tile_ownership_obj.get_player_id_from_cell(coords)
			var id2: int = tile_ownership_obj.get_player_id_from_cell(tile)
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
	var tile_ownership_obj: tile_ownership = tile_ownership.get_instance()
	for tile: Vector2i in all_network_coords:
		var id1: int = tile_ownership_obj.get_player_id_from_cell(coords)
		var id2: int = tile_ownership_obj.get_player_id_from_cell(tile)
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

func has_visited(visited: Dictionary, coords: Vector2i, direction: int) -> bool:
	return visited.has(coords) and visited[coords][direction]

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
	if number_of_trains >= 2:
		print("A")
	
	create_routes()

func create_routes() -> void:
	var train_manager_obj: train_manager = train_manager.get_instance()
	var i: int = 0
	var curr_train_id: int = train_members[i]
	
	var start_locations: Dictionary[int, rail_node] = {}
	
	#Removes old routes and starts each train up
	for id: int in train_members:
		var ai_train_obj: ai_train = train_manager_obj.get_ai_train(id)
		ai_train_obj.remove_all_stops()
		weight_serviced[id] = 0.0
		var node: rail_node = find_station_endnode()
		if node == null:
			node = get_biggest_node()
		service_node(node, id)
		ai_train_obj.add_stop.rpc(node.coords)
		start_locations[id] = node
		
	while true:
		#Kinda does weird stuff and doesn't pathfind quite right, check later
		
		i = get_smallest_index()
		curr_train_id = train_members[i]
		var ai_train_obj: ai_train = train_manager_obj.get_ai_train(curr_train_id)
		var node: rail_node = find_closest_station_to_add_to_route1(start_locations[curr_train_id], ai_train_obj)
		
		if node == null:
			#potentially send start location to another stop
			pass
		else:
			start_locations[curr_train_id] = node
		
		if check_for_completion():
			break
	
	
	ensure_train_routes_have_overlap()
	

func find_closest_station_to_add_to_route1(start: rail_node, ai_train_obj: ai_train) -> rail_node:
	var pq: priority_queue = priority_queue.new()
	pq.insert_element(start, 0)
	
	#No a great solution
	var dist: Dictionary[rail_node, int] = {} #Int is distance in edges, not actual distance
	dist[start] = 0
	
	#might be issues with overriding visited but we'll see
	var visited: Dictionary[Vector2i, Array] = {}
	fill_visited(visited, start.coords)

	var dest: rail_node = null
	while !pq.is_empty():
		var current: rail_node = (pq.pop_top() as rail_node)
		#Make sure it needs to visit and doesn't already visit
		if current.weight > 0 and !current.does_service(ai_train_obj.id):
			#Checks to make sure that unclaimed stations are priorized, but still goes for close stations
			if dest != null and !current.is_serviced():
				if floor(dist[current] * 0.8) < dist[dest]:
					dest = current
				break
			elif dest == null:
				dest = current
				if !current.is_serviced():
					break
		elif current.weight > 0:
			#Revisiting old station, could add station or even return
			pass
			
		#Doesn't care if an edge is taken
		var directions_available_to_go: Array = visited[current.coords]
		for edge: rail_edge in current.get_best_connections(directions_available_to_go):
			var node: rail_node = edge.get_other_node(current)
			var in_dir: int = edge.get_in_dir_to_node(node)
			if has_visited(visited, node.coords, in_dir):
				continue
			
			dist[node] = dist[current] + 1
			pq.insert_element(node, dist[node])
			#If station, then it can leave in both directions
			if node.weight > 0:
				fill_visited(visited, node.coords)
			else:
				intialize_visited(visited, node.coords, in_dir)
	if dest == null:
		pass
		#Potentially, add system that will try to look for a route from the trains[stop1] as start
		#Then add the new dest to the front of ai_train_obj, not at the back
	
	if dest != null:
		service_node(dest, ai_train_obj.id)
		#If multiple stops added and the start is from the start then the stop should be added to the start of stops
		if ai_train_obj.stops.size() > 2 and ai_train_obj.get_first_stop() == start.coords:
			ai_train_obj.add_stop.rpc(dest.coords, true)
		else:
			ai_train_obj.add_stop.rpc(dest.coords)
	return dest

func ensure_train_routes_have_overlap() -> void:
	if train_members.size() <= 1:
		return
	
	var ds_set: disjoint_set = create_route_ds_set()
	
	var biggest_subnetwork_id: int = ds_set.get_biggest_parent_id()
	#Gets the disjointed ids
	for id: int in train_members:
		var parent_id: int = ds_set.get_parent_index(id)
		if parent_id != biggest_subnetwork_id:
			#Is disjointed
			var node_added: rail_node = add_overlap(id)
			if node_added != null:
				#Should also update other disjointed trains in same set, so they wont have to conenct
				check_overlap_for_node(node_added, ds_set)

func create_route_ds_set() -> disjoint_set:
	#Do add overlap thing
	var ds_set: disjoint_set = disjoint_set.new()
	for id: int in train_members:
		ds_set.add_member(id)
	
	for node: rail_node in network.values():
		check_overlap_for_node(node, ds_set)
	return ds_set

func check_overlap_for_node(node: rail_node, ds_set: disjoint_set) -> void:
	if node.weight == 0:
		return
	var ids: Array = node.serviced_by.keys()
	var first: int = ids[0]
	for index: int in range(1, ids.size()):
		var curr: int = ids[index]
		ds_set.union(first, curr)

#Returns node overlapped to
func add_overlap(train_id: int) -> rail_node:
	var ai_train_obj: ai_train = train_manager.get_instance().get_ai_train(train_id)
	#Surveys for closest station owned by other node
	var start: rail_node = get_rail_node(ai_train_obj.get_last_stop())
	
	var node_added: rail_node = find_closest_station_to_add_to_route1(start, ai_train_obj)
	if node_added == null:
		start = get_rail_node(ai_train_obj.get_first_stop())
		node_added = find_closest_station_to_add_to_route1(start, ai_train_obj)
	return node_added

func find_station_endnode() -> rail_node:
	for node: rail_node in network.values():
		if node.connections.size() == 1 and node.weight > 0 and !node.is_serviced():
			return node
	#No endnodes
	return null

#Safe to call on already serviced nodes
func service_node(node: rail_node, train_id: int) -> void:
	if node.does_service(train_id):
		return
	var old_weight: float = node.get_relative_weight()
	node.service_node(train_id)
	var new_weight: float = node.get_relative_weight()
	for other_id: int in node.serviced_by:
		weight_serviced[other_id] -= old_weight
		weight_serviced[other_id] += new_weight
	
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
	var index: int = randi_range(0, weight_serviced_array.size() - 1) #Pick random for testing mostly
	var smallest: float = weight_serviced_array[index]
	for i: int in range(0, weight_serviced_array.size()):
		if smallest > weight_serviced_array[i]:
			index = i
			smallest = weight_serviced_array[i]
	return index

func check_for_completion() -> bool:
	for node: rail_node in network.values():
		#Only count stations
		if node.weight == 0:
			continue
		
		if !node.is_serviced():
			return false
	return true

func clear_ownership() -> void:
	for id: int in train_members:
		weight_serviced[id] = 0.0
	for node: rail_node in network.values():
		node.clear_ownership()

func start_trains() -> void:
	var train_manager_obj: train_manager = train_manager.get_instance()
	for train_id: int in train_members:
		train_manager_obj.get_ai_train(train_id).start_train.rpc()

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
