class_name map_data extends Node

#Singleton

var map: TileMapLayer

var depots: Dictionary[Vector2i, terminal] = {}

var holds: Dictionary = {}

var provinces: Dictionary = {}

var tiles_to_province_id: Dictionary = {}

var mutex: Mutex = Mutex.new()
static var singleton_instance: map_data

func _init(new_map: TileMapLayer) -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self
	map = new_map

static func get_instance() -> map_data:
	assert(singleton_instance != null, "Map_Data has not be created, and has been accessed")
	return singleton_instance

func add_depot(coords: Vector2i, depot: terminal) -> void:
	depots[coords] = depot

func get_depot(coords: Vector2i) -> terminal:
	if is_depot(coords):
		return depots[coords]
	return null

func get_depot_name(coords: Vector2i) -> String:
	if is_depot(coords):
		return depots[coords].get_depot_name()
	return ""

func is_depot(coords: Vector2i) -> bool:
	return depots.has(coords)

func is_owned_depot(coords: Vector2i, id: int) -> bool:
	return depots.has(coords) and depots[coords].get_player_owner() == id

func add_hold(coords: Vector2i, hold_name: String, player_id: int) -> void:
	mutex.lock()
	holds[coords] = [hold_name, player_id]
	mutex.unlock()

func get_hold_name(coords: Vector2i) -> String:
	if holds.has(coords):
		return holds[coords][0]
	return ""

func is_hold(coords: Vector2i) -> bool:
	return holds.has(coords)

func is_owned_hold(coords: Vector2i, id: int) -> bool:
	return holds.has(coords) and holds[coords][1] == id

func create_new_province() -> int:
	mutex.lock()
	var province_id: int = provinces.size()
	provinces[province_id] = province.new(province_id)
	mutex.unlock()
	return province_id

func create_new_if_empty(province_id: int) -> void:
	mutex.lock()
	if !provinces.has(province_id):
		provinces[province_id] = province.new(province_id)
	mutex.unlock()

func add_tile_to_province(province_id: int, tile: Vector2i) -> void:
	assert(!tiles_to_province_id.has(tile))
	mutex.lock()
	tiles_to_province_id[tile] = province_id
	provinces[province_id].add_tile(tile)
	mutex.unlock()

func add_many_tiles_to_province(province_id: int, tiles: Array) -> void:
	for tile: Vector2i in tiles:
		add_tile_to_province(province_id, tile)

func add_population_to_province(tile: Vector2i, pop: int) -> void:
	var id: int = get_province_id(tile)
	get_province(id).population += pop

func get_province_population(tile: Vector2i) -> int:
	var id: int = get_province_id(tile)
	return get_province(id).population

func get_population(province_id: int) -> int:
	return get_province(province_id).population

func is_tile_a_province(tile: Vector2i) -> bool:
	mutex.lock()
	var toReturn: bool = tiles_to_province_id.has(tile)
	mutex.unlock()
	return toReturn

func get_province_id(tile: Vector2i) -> int:
	mutex.lock()
	var toReturn: int = tiles_to_province_id[tile]
	mutex.unlock()
	return toReturn

func get_province(province_id: int) -> province:
	mutex.lock()
	var toReturn: province = provinces[province_id] 
	mutex.unlock()
	return toReturn
