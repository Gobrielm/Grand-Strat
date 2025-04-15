class_name province extends Node

var province_id: int
var population: int
var tiles: Array

func _init(_province_id: int) -> void:
	province_id = _province_id
	population = 0
	tiles = []

func add_tile(coords: Vector2i) -> void:
	tiles.append(coords)

func set_population(new_pop: int) -> void:
	population = new_pop

func get_tiles() -> Array:
	return tiles

func get_random_tile() -> Vector2i:
	return tiles.pick_random()
