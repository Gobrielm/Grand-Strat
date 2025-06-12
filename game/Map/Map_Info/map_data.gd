class_name map_data extends Node

#Singleton

var map: TileMapLayer

var depots: Dictionary[Vector2i, Terminal] = {}

var holds: Dictionary = {}

var provinces: Dictionary[int, Province] = {}

var tiles_to_province_id: Dictionary = {}

var country_id_to_province_ids: Dictionary[int, Dictionary] = {}

var mutex: Mutex = Mutex.new()
static var singleton_instance: map_data

func _init(new_map: TileMapLayer) -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self
	map = new_map

static func get_instance() -> map_data:
	assert(singleton_instance != null, "Map_Data has not be created, and has been accessed")
	return singleton_instance

# === Terminal Checks ===

func add_depot(coords: Vector2i, depot: Terminal) -> void:
	mutex.lock()
	depots[coords] = depot
	mutex.unlock()

@rpc("authority", "call_remote", "unreliable")
func client_add_depot(coords: Vector2i, p_owner: int) -> void:
	depots[coords] = vehicle_depot.new(coords, p_owner)

func get_depot(coords: Vector2i) -> Terminal:
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

# === Creators ===

func create_new_province() -> int:
	mutex.lock()
	var province_id: int = provinces.size()
	provinces[province_id] = Province.create(province_id)
	mutex.unlock()
	return province_id

func create_new_if_empty(province_id: int) -> void:
	mutex.lock()
	if !provinces.has(province_id):
		provinces[province_id] = Province.create(province_id)
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

# === Pop stuff ===

func add_population_to_province(tile: Vector2i, pop: int) -> void:
	var id: int = get_province_id(tile)
	get_province(id).add_population(pop)

func get_province_population(tile: Vector2i) -> int:
	var id: int = get_province_id(tile)
	return get_province(id).get_population()

func get_population(province_id: int) -> int:
	return get_province(province_id).get_population()

func get_population_as_level(province_id: int) -> int:
	@warning_ignore("integer_division")
	return get_province(province_id).get_population() / 50000

func get_total_population() -> int:
	var total: int = 0
	for prov: Province in provinces.values():
		total += prov.get_population()
	return total

func create_pops() -> void:
	var start: float = Time.get_ticks_msec()
	var threads: Array[Thread] = []
	var provs: Array = provinces.values()
	var amount_of_threads: int = 4
	var thread: Thread
	for i: int in range(amount_of_threads):
		thread = Thread.new()
		@warning_ignore("integer_division")
		thread.start(create_pops_range.bind(i * (provs.size() / amount_of_threads), (i + 1) * (provs.size() / amount_of_threads), provs))
		threads.push_back(thread)
	
	for thrd: Thread in threads:
		thrd.wait_to_finish()
	var end: float = Time.get_ticks_msec()
	print(str((end - start) / 1000) + " Seconds passed to create pops")

func create_pops_range(i: int, j: int, provs: Array) -> void:
	for index: int in range(i, j):
		var prov: Province = provs[index]
		prov.create_pops()

# === province Checks ===
func get_provinces() -> Array[Province]:
	var toReturn: Array[Province] = []
	toReturn.assign(provinces.values())
	return toReturn

func is_tile_a_province(tile: Vector2i) -> bool:
	mutex.lock()
	var toReturn: bool = tiles_to_province_id.has(tile)
	mutex.unlock()
	return toReturn

func get_province_id(tile: Vector2i) -> int:
	var toReturn: int = -1
	mutex.lock()
	if tiles_to_province_id.has(tile):
		toReturn = tiles_to_province_id[tile]
	mutex.unlock()
	return toReturn

func get_province(province_id: int) -> Province:
	var toReturn: Province = null
	mutex.lock()
	if provinces.has(province_id):
		toReturn = provinces[province_id] 
	mutex.unlock()
	return toReturn

func add_province_to_country(prov: Province, country_id: int) -> void:
	var old_id: int = prov.get_country_id()
	#Gets rid of it from old country
	if old_id != -1:
		country_id_to_province_ids[old_id].erase(prov.get_province_id())
	prov.set_country_id(country_id)
	if !country_id_to_province_ids.has(country_id):
		country_id_to_province_ids[country_id] = {}
	country_id_to_province_ids[country_id][prov.get_province_id()] = true

func get_counties_provinces(country_id: int) -> Dictionary:
	return country_id_to_province_ids[country_id]

func get_province_terminal_tiles(province: Province) -> Array:
	var toReturn: Array = []
	mutex.lock()
	toReturn = province.get_terminal_tiles()
	mutex.unlock()
	return toReturn

func get_province_rand_tile(province: Province) -> Vector2i:
	var toReturn: Vector2i
	mutex.lock()
	toReturn = province.get_random_tile()
	mutex.unlock()
	return toReturn
