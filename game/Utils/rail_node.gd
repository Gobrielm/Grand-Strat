class_name rail_node extends Node

var coords: Vector2i
var weight: float
var serviced_by: Dictionary[int, bool] = {}
var connections: Dictionary[rail_node, sorted_stack] = {}

func _init(p_coords: Vector2i, p_weight: float) -> void:
	coords = p_coords
	weight = p_weight

func get_weight() -> float:
	return weight / serviced_by.size()

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

func get_best_connection(other_node: rail_node) -> rail_edge:
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
	var edge: rail_edge = get_best_connection(other_node)
	edge.claim_edge(train_id)

func get_only_connected_node() -> Vector2i:
	return connections.keys()[0]

func get_biggest_node() -> rail_node:
	var biggest: rail_node = null
	for node: rail_node in connections.keys():
		if biggest == null or biggest.weight < node.weight:
			biggest = node
	return biggest

func get_best_edge() -> rail_edge:
	var closest: rail_edge = null
	for node: rail_node in connections:
		for val: weighted_value in connections[node].backing_array:
			var edge: rail_edge = val.val
			if !edge.is_edge_claimed() and (closest == null or val.weight > closest.weight):
				closest = val.val
				break
	return closest
