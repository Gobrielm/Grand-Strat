class_name rail_edge extends Node

var node1: rail_node
var node2: rail_node
var out_dir1: int
var out_dir2: int

var weight: float
var serviced_by: Dictionary[int, bool]

func _init(p_node1: rail_node, p_node2: rail_node, p_weight: float, p_out_dir1: int, p_out_dir2: int) -> void:
	node1 = p_node1
	node2 = p_node2
	weight = p_weight
	out_dir1 = p_out_dir1
	out_dir2 = p_out_dir2
	serviced_by = {}

func is_edge_claimed_by_id(train_id: int) -> bool:
	return serviced_by.has(train_id)

func is_edge_claimed() -> bool:
	return serviced_by.size() > 0

func claim_edge(train_id: int) -> void:
	serviced_by[train_id] = true

func is_traversable(input_dir: int, turning_node: rail_node) -> bool:
	var dirs1: Array[int] = [(out_dir1) % 6, (out_dir1 + 1) % 6, (out_dir1 + 5) % 6]
	var dirs2: Array[int] = [(out_dir2) % 6, (out_dir2 + 1) % 6, (out_dir2 + 5) % 6]
	return (turning_node == node1 and input_dir in dirs1) or (turning_node == node2 and input_dir in dirs2)

func get_out_dir_from_node(node: rail_node) -> int:
	return out_dir1 if node == node1 else out_dir2

func get_in_dir_to_node(node: rail_node) -> int:
	return (out_dir1 + 3) % 6 if node == node1 else (out_dir2 + 3) % 6

func get_other_direction(node: rail_node) -> int:
	return out_dir2 if node == node1 else out_dir1

func get_other_node(node: rail_node) -> rail_node:
	return node2 if node1 == node else node1

func copy() -> rail_edge:
	return rail_edge.new(node1, node2, weight, out_dir1, out_dir2)

func clear_ownership() -> void:
	serviced_by.clear()

func _to_string() -> String:
	var toReturn: String = node1.to_string_no_edges() + ": " + str(out_dir1) + "->" + str(out_dir2) + " :" + node2.to_string_no_edges()
	if is_edge_claimed():
		toReturn += " x"
	return toReturn
