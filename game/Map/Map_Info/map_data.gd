class_name map_data extends Node

#Singleton

var map: TileMapLayer

var depots: Dictionary[Vector2i, Terminal] = {}

var holds: Dictionary = {}

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
