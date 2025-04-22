class_name rail_edge extends Node

var node1: rail_node
var node2: rail_node
var weight: float

func _init(p_node1: rail_node, p_node2: rail_node, p_weight: float) -> void:
	node1 = p_node1
	node2 = p_node2
	weight = p_weight
