class_name state extends Node

var provinces: Array[int] = []
var state_id: int

func _init(_state_id: int) -> void:
	state_id = _state_id

func add_province(prov_id: int) -> void:
	provinces.push_back(prov_id)

func get_provinces() -> Array[int]:
	return provinces
