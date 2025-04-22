class_name rail_node extends Node

var coords: Vector2i
var weight: float
var serviced_by: Array[int] = []
var connections: Dictionary[rail_node, sorted_stack] = {}

func _init(p_coords: Vector2i, p_weight: float) -> void:
	coords = p_coords
	weight = p_weight

func get_weight() -> float:
	return weight / serviced_by.size()

func connect_nodes(p_node: rail_node, p_weight: float) -> void:
	if !connections.has(p_node):
		connections[p_node] = sorted_stack.new()
	connections[p_node].insert_element(0, p_weight)

func service_node(train_id: int) -> void:
	if !does_service(train_id):
		serviced_by.append(train_id)

func does_service(train_id: int) -> bool:
	for id: int in serviced_by:
		if id == train_id:
			return true
	return false

func is_serviced() -> bool:
	return serviced_by.size() > 0

func is_connection_claimed(other_node: rail_node, p_weight: float) -> bool:
	for element: weighted_value in connections[other_node].backing_array:
		if element.weight == p_weight and element.val == 0:
			return true
	return false

func get_best_connection(other_node: rail_node) -> float:
	var backing_array: Array = connections[other_node].backing_array
	for element: weighted_value in backing_array:
		if element.val == 0:
			return element.weight
	return backing_array[0].weight

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

func claim_best_connection(other_node: rail_node) -> void:
	var p_weight: float = get_best_connection(other_node)
	claim_connection(other_node, p_weight)

func get_only_connected_node() -> Vector2i:
	return connections.keys()[0]

func get_biggest_node() -> rail_node:
	var biggest: rail_node = null
	for node: rail_node in connections.keys():
		if biggest == null or biggest.weight < node.weight:
			biggest = node
	return biggest

func get_best_edge() -> rail_edge:
	var closest: rail_node = null
	var route_weight: float = 0.0
	for node: rail_node in connections:
		for val: weighted_value in connections[node].backing_array:
			if val.val == 0 and (closest == null or val.weight > route_weight):
				closest = node
				route_weight = val.weight
				break
	if closest == null:
		return null
	return rail_edge.new(self, closest, route_weight)
