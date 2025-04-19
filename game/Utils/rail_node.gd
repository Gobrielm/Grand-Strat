class_name rail_node extends Node

var coords: Vector2i
var connections: Dictionary[Vector2i, sorted_stack] = {}

func _init(p_coords: Vector2i) -> void:
	coords = p_coords

func connect_nodes(p_coords: Vector2i, weight: float) -> void:
	if !connections.has(p_coords):
		connections[p_coords] = sorted_stack.new()
	connections[p_coords].insert_element(weight, weight)
