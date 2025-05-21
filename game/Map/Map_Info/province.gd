class_name province extends Node

var province_id: int
var population: int
var tiles: Array
var terminal_tiles: Dictionary[Vector2i, terminal]
var country_id: int = -1

func _init(_province_id: int) -> void:
	province_id = _province_id
	population = 0
	tiles = []
	terminal_tiles = {}

func add_tile(coords: Vector2i) -> void:
	tiles.append(coords)

func set_population(new_pop: int) -> void:
	population = new_pop

func set_country_id(p_country_id: int) -> void:
	country_id = p_country_id

func get_tiles() -> Array:
	return tiles

func get_random_tile() -> Variant:
	var temp_tiles: Array = tiles.duplicate()
	var rand_index: int = randi() % temp_tiles.size()
	var random_tile: Vector2i = temp_tiles.pop_at(rand_index)
	while terminal_tiles.has(random_tile):
		if temp_tiles.size() == 0:
			#Will probs never get called
			return Vector2i(0, 0)
		rand_index = randi() % temp_tiles.size()
		random_tile = temp_tiles.pop_at(rand_index)
	return random_tile

func add_terminal(coords: Vector2i, term: terminal) -> void:
	terminal_tiles[coords] = term

func remove_terminal(coords: Vector2i) -> void:
	#BUG: Never gets called when deleting terminals
	terminal_tiles.erase(coords)

func get_terminals() -> Array[terminal]:
	return terminal_tiles.values()
