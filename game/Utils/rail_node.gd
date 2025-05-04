class_name rail_node extends Node

var coords: Vector2i
var weight: float
var serviced_by: Dictionary[int, bool] = {}
var connections: Dictionary[rail_node, sorted_stack] = {}

func _init(p_coords: Vector2i, p_weight: float) -> void:
	coords = p_coords
	weight = p_weight

func get_relative_weight() -> float:
	if serviced_by.size() == 0:
		return 0
	return weight / serviced_by.size()

func get_weight() -> float:
	return weight

func connect_nodes(edge: rail_edge) -> void:
	var p_node: rail_node = edge.node1 if edge.node1 != self else edge.node2
	if !connections.has(p_node):
		connections[p_node] = sorted_stack.new()
	connections[p_node].insert_element(edge, edge.weight)

func service_node(train_id: int) -> void:
	if !does_service(train_id):
		serviced_by[train_id] = true

func does_service(train_id: int) -> bool:
	return serviced_by.has(train_id)

func is_serviced() -> bool:
	return serviced_by.size() > 0

func get_best_unowned_connection(other_node: rail_node) -> rail_edge:
	var backing_array: Array = connections[other_node].backing_array
	for element: weighted_value in backing_array:
		var edge: rail_edge = element.val
		if !edge.is_edge_claimed():
			return edge
	return backing_array[0].val

func claim_connection(other_node: rail_node, p_weight: float) -> void:
	var temp: weighted_value
	for element: weighted_value in connections[other_node].backing_array:
		if element.weight == p_weight:
			if element.val > 0:
				temp = element
			else:
				element.val = 1
				return
	if temp != null:
		temp.val += 1

func claim_best_connection(other_node: rail_node, train_id: int) -> void:
	var edge: rail_edge = get_best_unowned_connection(other_node)
	edge.claim_edge(train_id)

func get_only_connected_node() -> rail_node:
	return (connections.keys()[0] as rail_node)

func get_best_connections(output_directions: Array) -> Array[rail_edge]:
	var toReturn: Array[rail_edge] = []
	for node: rail_node in connections:
		var edge: rail_edge = get_best_unowned_connection(node)
		for dir: int in range(0, 6):
			if output_directions[dir] and edge.is_traversable(dir, self):
				toReturn.push_back(edge)
				break
	return toReturn

func get_biggest_node() -> rail_node:
	var biggest: rail_node = null
	for node: rail_node in connections.keys():
		if biggest == null or biggest.weight < node.weight:
			biggest = node
	return biggest

func get_best_edge(train_id: int) -> rail_edge:
	var closest: rail_edge = null
	#Using independent weight as it will be altered
	var best_weight: float = -1.0
	for node: rail_node in connections:
		for val: weighted_value in connections[node].backing_array:
			var edge: rail_edge = val.val
			var e_weight: float = edge.weight
			#If already service, then reduce weight, but still include as it could be neccessary for pathfinding
			if node.does_service(train_id):
				e_weight /= 5
			if !can_reach_connection(train_id, edge) or edge.is_edge_claimed():
				continue
			if closest == null or e_weight > best_weight:
				closest = edge
				best_weight = e_weight
				break
	return closest

func can_reach_connection(train_id: int, connection: rail_edge) -> bool:
	#ISSUES here or with adding outout dir1, dir2
	for stack: sorted_stack in connections.values():
		for w_val: weighted_value in stack.backing_array:
			var edge: rail_edge = w_val.val
			if !edge.is_edge_claimed_by_id(train_id):
				continue
			#Gets input direction from claimed edge
			var dir: int = (edge.get_out_dir_from_node(self) + 3) % 6
			#Gets output direction from connection to be made
			var other_dir: int = connection.get_out_dir_from_node(self)
			#If they match then a connection is possible
			if (dir == other_dir or (dir + 1) % 6 == other_dir or (dir + 5) % 6 == other_dir):
				return true
	return false

func get_connects_to_owned_nodes(train_id: int) -> int:
	var count: int = 0
	for node: rail_node in connections:
		if node.does_service(train_id):
			count += 1
	return count

func get_owned_connected_nodes(train_id: int) -> Array[rail_node]:
	var toReturn: Array[rail_node] = []
	for node: rail_node in connections:
		for w_val: weighted_value in connections[node].backing_array:
			var edge: rail_edge = (w_val.val as rail_edge)
			if edge.is_edge_claimed_by_id(train_id):
				toReturn.append(node)
				break
	return toReturn

func get_owned_edges(train_id: int) -> Array[rail_edge]:
	var toReturn: Array[rail_edge] = []
	for node: rail_node in connections:
		for w_val: weighted_value in connections[node].backing_array:
			var edge: rail_edge = (w_val.val as rail_edge)
			if edge.is_edge_claimed_by_id(train_id):
				toReturn.append(edge)
	return toReturn

func get_edges() -> Array[rail_edge]:
	var toReturn: Array[rail_edge] = []
	for stack: sorted_stack in connections.values():
		var backing_array: Array = stack.backing_array
		for w_val: weighted_value in backing_array:
			toReturn.append(w_val.val as rail_edge)
	return toReturn

func clear_ownership() -> void:
	serviced_by.clear()
	for edge: rail_edge in get_edges():
		edge.clear_ownership()

func to_string_no_edges() -> String:
	return str(coords)

func _to_string() -> String:
	var toReturn: String = to_string_no_edges()
	for node: rail_node in connections:
		toReturn += " -"
		for val: weighted_value in connections[node].backing_array:
			#Transforms into distance
			toReturn += str((10 / (val.val as rail_edge).weight) as int)  + ", "
		toReturn = toReturn.trim_suffix(", ") #Cuts extra comma
		toReturn += "> " + node.to_string_no_edges() + ", "
	toReturn += "\n-----------------------\n"
	return toReturn
