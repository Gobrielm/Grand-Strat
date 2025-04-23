class_name rail_network extends Node

var network_id: int
var network: Dictionary[Vector2i, rail_node]
var weight_serviced: Dictionary[int, float] #Train id -> weight serviced
var train_members: Array[int] #Train id -> bool for set

func _init() -> void:
	network = {}
	weight_serviced = {}
	train_members = []

func clear_network() -> void:
	network.clear()
	for id: int in train_members:
		weight_serviced[id] = 0.0

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

func create_network(start: Vector2i) -> void:
	#TODO: Clears when gets created
	clear_network()
	var map: TileMapLayer = Utils.world_map
	var queue: Array = [start]
	var visited: Dictionary = {} # Vector2i -> Array[Bool for each direction]
	var dist: Dictionary[Vector2i, rail_info] = {} #Array[Vector2i(Source), int(dist)]
	visited[start] = Utils.rail_placer.get_track_connections(start)
	var curr: Vector2i
	while !queue.is_empty():
		curr = queue.pop_front()
		#var is_node: bool = terminal_map.is_station(curr) or map_data.get_instance().is_depot(curr) or Utils.rail_placer.get_track_connection_count(curr) >= 3
		#if is_node:
			#create_node(curr)
			#if dist.has(curr):
				##rail_info.new(dist[curr].source, dist[curr].dist, dist[curr].input_dir)
				#connect_nodes(curr, dist[curr][0], dist[curr][1])
			#dist[curr] = [curr, 0]
		
		var cells_to_check: Array = get_cells_in_front(curr, visited[curr], map)
		for direction: int in cells_to_check.size():
			var tile: Vector2i = cells_to_check[direction]
			if tile != null and map.do_tiles_connect(curr, tile) and !check_visited(visited, tile, direction):
				intialize_visited(visited, tile, direction)
				queue.push_back(tile)
				dist[tile] = dist[curr]
				dist[tile].dist += 1
				if dist[tile].dist == 1:
					dist[tile].output_dir = direction
				
				var is_node: bool = terminal_map.is_station(tile) or map_data.get_instance().is_depot(tile) or Utils.rail_placer.get_track_connection_count(tile) >= 3
				if is_node:
					create_node(tile)
					if dist.has(tile):
						connect_nodes(tile, dist[curr].source, dist[curr].dist, (direction + 3) % 6, dist[curr].output_dir)
					dist[curr] = rail_info.new(curr, 0, -1)

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

func check_visited(visited: Dictionary, coords: Vector2i, direction: int) -> bool:
	if visited.has(coords):
		return visited[coords][direction]
	return false

func create_node(coords: Vector2i) -> void:
	if !network.has(coords):
		var weight: int = 0
		var stat: station = terminal_map.get_station(coords)
		if stat != null:
			weight = stat.get_orders_magnitude()
		network[coords] = rail_node.new(coords, weight)

func connect_nodes(coords1: Vector2i, coords2: Vector2i, dist: int, output_dir1: int, output_dir2: int) -> void:
	if network.has(coords1) and network.has(coords2):
		var node1: rail_node = network[coords1]
		var node2: rail_node = network[coords2]
		#TODO: Add directions for edges
		var edge: rail_edge = rail_edge.new(node1, node2, 10.0 / dist, output_dir1, output_dir2)
		node1.connect_nodes(edge)
		node2.connect_nodes(edge)

func get_rail_node(coords: Vector2i) -> rail_node:
	return network[coords]

func split_up_network_among_trains() -> void:
	#Network cannot be completed
	if network.size() <= 1:
		return
	
	var number_of_trains: int = get_number_of_trains()
	var i: int = 0
	var curr_train_id: int = train_members[i]
	#Creates endnode connections
	while i != 0:
		var endnode: rail_node = find_endnode()
		if endnode == null:
			#No unclaimed endnodes, rest will be largest unclaimed
			break
		service_node(endnode, curr_train_id)
		var other_coords: Vector2i = endnode.get_only_connected_node()
		var other_node: rail_node = network[other_coords]
		endnode.claim_best_connection(other_node, curr_train_id)
		service_node(other_node, curr_train_id)
		
		i = (i + 1) % number_of_trains
		curr_train_id = train_members[i]
	
	#Fills in rest around the biggest stations
	while i != 0:
		var node: rail_node = get_biggest_node()
		service_node(node, curr_train_id)
		var other_node: rail_node = node.get_biggest_node()
		node.claim_best_connection(other_node, curr_train_id)
		service_node(other_node, curr_train_id)
		i = (i + 1) % number_of_trains
		curr_train_id = train_members[i]
	#No ai_train will have more than 1 node by this point
	#Will run til all nodes completed
	while true:
		i = get_smallest_index()
		curr_train_id = train_members[i]
		var edge: rail_edge = get_best_edge_in_network(curr_train_id)
		edge.claim_edge(curr_train_id)
		service_node(edge.node1, curr_train_id)
		service_node(edge.node2, curr_train_id)
		if check_for_completion():
			break
		

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
		if (biggest == null and node.weight > 0) or (node.weight / (node.serviced_by.size() + 1)) > biggest.weight:
			biggest = node
	return biggest

func get_smallest_index() -> int:
	var smallest: float = weight_serviced[0]
	var index: int = 0
	for i: int in range(1, weight_serviced.size()):
		if smallest > weight_serviced[i]:
			index = i
			smallest = weight_serviced[i]
	return index
	
func get_best_edge_in_network(train_id: int) -> rail_edge:
	var best_edge: rail_edge = null
	for node: rail_node in network.values():
		#Only include nodes that are serviced by train
		if node.does_service(train_id):
			var edge: rail_edge = node.get_best_edge()
			if best_edge == null or edge.weight > best_edge.weight:
				best_edge = edge
	return best_edge
	
func check_for_completion() -> bool:
	for node: rail_node in network.values():
		#Only count stations
		if node.get_weight() > 0 and !node.is_serviced():
			return false
	return true
