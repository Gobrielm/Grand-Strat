extends Node

var map: TileMapLayer

var depots = {}

var holds = {}

var provinces := {}

var tiles_to_province_id := {}

func _init(new_map):
	map = new_map
	Utils.assign_tile_info(self)

func add_depot(coords: Vector2i, depot: terminal, player_id: int):
	depots[coords] = [depot, player_id]

func get_depot(coords: Vector2i) -> terminal:
	if is_depot(coords):
		return depots[coords][0]
	return null

func get_depot_name(coords: Vector2i) -> String:
	if is_depot(coords):
		return depots[coords][0].get_depot_name()
	return ""

func is_depot(coords: Vector2i) -> bool:
	return depots.has(coords)

func is_owned_depot(coords: Vector2i, id: int) -> bool:
	return depots.has(coords) and depots[coords][1] == id

func add_hold(coords: Vector2i, hold_name: String, player_id: int):
	holds[coords] = [hold_name, player_id]

func get_hold_name(coords: Vector2i) -> String:
	if holds.has(coords):
		return holds[coords][0]
	return ""

func is_hold(coords: Vector2i) -> bool:
	return holds.has(coords)

func is_owned_hold(coords: Vector2i, id: int) -> bool:
	return holds.has(coords) and holds[coords][1] == id

func create_new_province() -> int:
	var province_id = provinces.size()
	provinces[province_id] = province.new(province_id)
	return province_id

func create_new_if_empty(province_id: int):
	if !provinces.has(province_id):
		provinces[province_id] = province.new(province_id)

func add_tile_to_province(province_id: int, tile: Vector2i):
	assert(!tiles_to_province_id.has(tile))
	tiles_to_province_id[tile] = province_id
	provinces[province_id].add_tile(tile)

func add_many_tiles_to_province(province_id: int, tiles: Array):
	for tile in tiles:
		add_tile_to_province(province_id, tile)

func add_population_to_province(tile: Vector2i, pop: int):
	var id := get_province_id(tile)
	get_province(id).population += pop

func get_province_population(tile: Vector2i) -> int:
	var id: int = get_province_id(tile)
	return get_province(id).population

func is_tile_a_province(tile: Vector2i) -> bool:
	return tiles_to_province_id.has(tile)

func get_province_id(tile: Vector2i) -> int:
	return tiles_to_province_id[tile]

func get_province(province_id: int) -> province:
	return provinces[province_id]
