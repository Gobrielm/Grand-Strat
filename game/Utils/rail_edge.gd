class_name rail_edge extends Node

var node1: rail_node
var node2: rail_node
var weight: float
var serviced_by: Dictionary[int, bool]

func _init(p_node1: rail_node, p_node2: rail_node, p_weight: float) -> void:
	node1 = p_node1
	node2 = p_node2
	weight = p_weight
	serviced_by = {}

func is_edge_claimed(train_id: int) -> bool:
	return serviced_by.has(train_id)

func claim_edeg(train_id: int) -> void:
	serviced_by[train_id] = true
