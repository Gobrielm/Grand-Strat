class_name province extends Node

var province_id: int
var population: int
var tiles: Array

func _init(_province_id: int):
	province_id = _province_id
	population = 0
	tiles = []

func add_tile(coords: Vector2i):
	tiles.append(coords)

func set_population(new_pop: int):
	population = new_pop
