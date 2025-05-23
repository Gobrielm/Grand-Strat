class_name rail_placer extends Node2D

static var singleton_instance: rail_placer

var depot_script: GDScript = preload("res://Cargo/vehicle_depot.gd")

var mutex: Mutex = Mutex.new()
var temp_layer_array: Array[TileMapLayer] = []
var track_connection: Dictionary[Vector2i, Array] = {}
const orientation_vectors: Array[Vector2] = [
	Vector2(0, 1),
	Vector2(sqrt(3)/2, 0.5),
	Vector2(sqrt(3)/2, -0.5),
	Vector2(0, -1),
	Vector2(-sqrt(3)/2, -0.5),
	Vector2(-sqrt(3)/2, 0.5)
]

var orientation: int = 0
var type: int = -1
var old_coordinates: Vector2i

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> rail_placer:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

@onready var rail_layer_0: TileMapLayer = $Rail_Layer_0
@onready var rail_layer_1: TileMapLayer = $Rail_Layer_1
@onready var rail_layer_2: TileMapLayer = $Rail_Layer_2
@onready var rail_layer_3: TileMapLayer = $Rail_Layer_3
@onready var rail_layer_4: TileMapLayer = $Rail_Layer_4
@onready var rail_layer_5: TileMapLayer = $Rail_Layer_5

func _ready() -> void:
	for item: Node in get_children():
		if item.name.begins_with("Rail_Temp_Layer"):
			temp_layer_array.append(item as TileMapLayer)

func init_all_rails() -> void:
	for cell: Vector2i in Utils.world_map.get_used_cells():
		init_track_connection(cell)

func get_rail_layers() -> Array[TileMapLayer]:
	var toReturn: Array[TileMapLayer] = []
	mutex.lock()
	for num: int in range(6):
		toReturn.append(get_rail_layer(num))
	mutex.unlock()
	return toReturn

func hover_tile(coordinates: Vector2i, coords_middle: Vector2, coords_mouse: Vector2) -> void:
	delete_hover_rail()
	old_coordinates = coordinates
	var pointer: Vector2 = (coords_mouse - coords_middle).normalized()
	pointer.y *= -1
	var smallest_dist: float = 10000.0
	for index: int in orientation_vectors.size():
		var vector: Vector2 = orientation_vectors[index]
		var dist: float = abs(pointer.distance_to(vector))
		if dist < smallest_dist:
			smallest_dist = dist
			orientation = index
	if orientation == -1:
		return
	var temp_layer: TileMapLayer = get_temp_layer(orientation)
	temp_layer.set_cell(coordinates, 0, Vector2i(orientation, type))

func hover(coordinates: Vector2i, new_type: int, coords_middle: Vector2, coords_mouse: Vector2) -> void:
	type = new_type
	hover_tile(coordinates, coords_middle, coords_mouse)

func hover_debug(coords: Vector2i, p_orientation: int) -> void:
	var temp_layer: TileMapLayer = get_temp_layer(p_orientation)
	temp_layer.set_cell(coords, 0, Vector2i(p_orientation, 0))

func hover_many_tiles(tiles: Dictionary) -> void:
	clear_all_temps()
	for coords: Vector2i in tiles:
		for tile: Vector2i in tiles[coords]:
			var temp_layer: TileMapLayer = get_temp_layer(tile.x)
			temp_layer.set_cell(coords, 0, tile)

func clear_all_real() -> void:
	track_connection.clear()
	for layer: Node in get_children():
		if layer.name.begins_with("Rail_Layer"):
			(layer as TileMapLayer).clear()

func clear_all_temps() -> void:
	for layer: TileMapLayer in temp_layer_array:
		layer.clear()

func place_depot(coords: Vector2i, player_id: int) -> void:
	encode_depot.rpc(coords, player_id)

@rpc("authority", "call_local", "unreliable")
func encode_depot(coords: Vector2i, player_id: int) -> void:
	map_data.get_instance().add_depot(coords, depot_script.new(coords, player_id))

func place_station(coords: Vector2i, p_orientation: int, player_id: int) -> void:
	create_map_tile.rpc(coords, (p_orientation + 3) % 6, 2)
	encode_station.rpc(coords, player_id)
	terminal_map.create_station(coords, player_id)

@rpc("authority", "call_local", "unreliable")
func encode_station(coords: Vector2i, new_owner: int) -> void:
	map_data.get_instance().add_hold(coords, "Station", new_owner)

#Should only be called as server
func place_tile(coords: Vector2i, p_orientation: int, p_type: int, player_id: int) -> void:
	#Checks if spot is taken, if tile is traversable, and player is in the country
	if is_already_built(coords, p_orientation) and Utils.world_map.is_tile_traversable(coords) and tile_ownership.get_instance().is_owned(player_id, coords):
		return
	add_track_connection(coords, p_orientation)
	create_map_tile.rpc(coords, p_orientation, p_type)
	if p_type == 1:
		place_depot(coords, player_id)
	elif p_type == 2:
		place_station(coords, p_orientation, player_id)

@rpc("authority", "call_local", "unreliable")
func create_map_tile(coords: Vector2i, new_orientation: int, new_type: int) -> void:
	var rail_layer: TileMapLayer = get_rail_layer(new_orientation)
	mutex.lock()
	rail_layer.set_cell(coords, 0, Vector2i(new_orientation, new_type))
	mutex.unlock()

func remove_tile(coords: Vector2i, new_orientation: int, _new_type: int) -> void:
	#TODO: Doesn't delete station, or depot, but only used safely in testing
	print("Testing function used, is potentially unsafe")
	var rail_layer: TileMapLayer = get_rail_layer(new_orientation)
	rail_layer.erase_cell(coords)
	delete_track_connection(coords, new_orientation)

func place_road_depot(coords: Vector2i, player_id: int) -> void:
	for i: int in 6:
		if is_already_built(coords, i):
			return
	var rail_layer: TileMapLayer = get_rail_layer(0)
	rail_layer.set_cell(coords, 0, Vector2i(0, 3))
	terminal_map.create_road_depot(coords, player_id)

func is_already_built(coords: Vector2i, new_orientation: int) -> bool:
	var rail_layer: TileMapLayer = get_rail_layer(new_orientation)
	assert(rail_layer != null)
	return rail_layer.get_cell_atlas_coords(coords).x == new_orientation

func delete_hover_rail() -> void:
	if old_coordinates != null and orientation != null:
		var temp_layer: TileMapLayer = get_temp_layer(orientation)
		temp_layer.erase_cell(old_coordinates)

func get_orientation() -> int:
	return orientation

func get_coordinates() -> Vector2i:
	return old_coordinates

func get_rail_layer(curr_orientation: int) -> TileMapLayer:
	if curr_orientation == 0:
		return rail_layer_0
	elif curr_orientation == 1:
		return rail_layer_1
	elif curr_orientation == 2:
		return rail_layer_2
	elif curr_orientation == 3:
		return rail_layer_3
	elif curr_orientation == 4:
		return rail_layer_4
	elif curr_orientation == 5:
		return rail_layer_5
	assert(false)
	return rail_layer_0 # default fallback

func get_temp_layer(curr_orientation: int) -> TileMapLayer:
	if curr_orientation >= 0 and curr_orientation < 6:
		return temp_layer_array[curr_orientation]
	return null

func init_track_connection(coords: Vector2i) -> void:
	mutex.lock()
	track_connection[coords] = [false, false, false, false, false, false]
	mutex.unlock()

func add_track_connection(coords: Vector2i, new_orientation: int) -> void:
	mutex.lock()
	if !track_connection.has(coords):
		mutex.unlock()
		init_track_connection(coords)
	else:
		mutex.unlock()
	mutex.lock()
	track_connection[coords][new_orientation] = true
	mutex.unlock()

func delete_track_connection(coords: Vector2i, new_orientation: int) -> void:
	mutex.lock()
	track_connection[coords][new_orientation] = false
	mutex.unlock()

func get_track_connections(coords: Vector2i) -> Array[bool]:
	var toReturn: Array[bool] = [false, false, false, false, false, false]
	mutex.lock()
	if track_connection.has(coords):
		toReturn.assign((track_connection[coords] as Array).duplicate())
	mutex.unlock()
	return toReturn

func get_depot_direction(coords: Vector2i) -> int:
	for rail_layer: TileMapLayer in get_rail_layers():
		var atlas: Vector2i = rail_layer.get_cell_atlas_coords(coords)
		if atlas.y == 1:
			return atlas.x
	return -1

func get_track_connection_count(coords: Vector2i) -> int:
	var count: int = 0
	for dir: bool in get_track_connections(coords):
		if dir:
			count += 1
	return count

func are_tiles_connected_by_rail(coord1: Vector2i, coord2: Vector2i, bordering_to_coord1: Array[Vector2i]) -> bool:
	var track_connections1: Array = get_track_connections(coord1)
	var track_connections2: Array = get_track_connections(coord2)
	for direction: int in bordering_to_coord1.size():
		var real_direction: int = (direction + 2) % 6
		if bordering_to_coord1[direction] == coord2:
			return track_connections1[real_direction] and track_connections2[(real_direction + 3) % 6]
	return false

func get_station_orientation(coords: Vector2i) -> int:
	for rail_layer: TileMapLayer in get_rail_layers():
		var atlas: Vector2i = thread_get_cell_atlas_coords(rail_layer, coords)
		if atlas.y == 2:
			return atlas.x % 3
	return -1

func thread_get_cell_atlas_coords(rail_layer: TileMapLayer, coords: Vector2i) -> Vector2i:
	mutex.lock()
	var toReturn: Vector2i = rail_layer.get_cell_atlas_coords(coords)
	mutex.unlock()
	return toReturn
