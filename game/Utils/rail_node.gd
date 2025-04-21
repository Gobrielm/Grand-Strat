class_name rail_node extends Node

var coords: Vector2i
var weight: int
var serviced_by: Array[int]
var connections: Dictionary[Vector2i, sorted_stack] = {}

func _init(p_coords: Vector2i, p_weight: int) -> void:
	coords = p_coords
	weight = p_weight

func connect_nodes(p_coords: Vector2i, p_weight: float) -> void:
	if !connections.has(p_coords):
		connections[p_coords] = sorted_stack.new()
	connections[p_coords].insert_element(0, p_weight)

func service_node(train_id: int) -> void:
	if !does_service(train_id):
		serviced_by.append(train_id)

func does_service(train_id: int) -> bool:
	for id: int in serviced_by:
		if id == train_id:
			return true
	return false

func is_connection_claimed(other_node: Vector2i, p_weight: int) -> bool:
	for element: weighted_value in connections[other_node].backing_array:
		if element.weight == p_weight and element.val == 0:
			return true
	return false

func get_best_connection(other_node: Vector2i) -> int:
	#TODO: Pick best weight that not owned
	return 0

func claim_connection(other_node: Vector2i, p_weight: int) -> void:
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
