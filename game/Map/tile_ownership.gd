class_name tile_ownership extends TileMapLayer

var tile_to_country_id: Dictionary = {}
var country_id_to_tiles_owned: Dictionary = {}

var country_id_to_player_id: Dictionary = {}
var player_id_to_country_id: Dictionary = {}

var mutex: Mutex = Mutex.new()

static var singleton_instance: tile_ownership = null

func _ready() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	call_deferred("create_countries")
	singleton_instance = self

static func get_instance() -> tile_ownership:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

func create_countries() -> void:
	var provinces_to_add: Array = [Vector2i(89, -112), Vector2i(105, -116), Vector2i(103, -105), Vector2i(103, -97), Vector2i(114, -114)]
	for cell: Vector2i in provinces_to_add:
		var prov: province = map_data.get_instance().get_province(map_data.get_instance().get_province_id(cell))
		for tile: Vector2i in prov.tiles:
			add_tile_to_country(tile, 1)

func add_tile_to_country(tile: Vector2i, country_id: int) -> void:
	if !country_id_to_tiles_owned.has(country_id):
		country_id_to_tiles_owned[country_id] = []
	country_id_to_tiles_owned[country_id].append(tile)
	tile_to_country_id[tile] = country_id
	@warning_ignore("integer_division")
	set_cell(tile, 0, Vector2i(country_id / 8, country_id % 8))

@rpc("authority", "call_local", "reliable")
func refresh_tile_ownership(_resource: Dictionary) -> void:
	pass

@rpc("any_peer", "call_local", "reliable")
func prepare_refresh_tile_ownership() -> void:
	var dict: Dictionary = {}
	for cell: Vector2i in get_used_cells():
		dict[cell] = get_cell_atlas_coords(cell)
	refresh_tile_ownership.rpc_id(multiplayer.get_remote_sender_id(), dict)

@rpc("any_peer", "call_local", "unreliable")
func add_player_to_country(player_id: int, coords: Vector2i) -> void:
	if !tile_to_country_id.has(coords):
		return
	var country_id: int = tile_to_country_id[coords]
	
	if !country_id_to_player_id.has(country_id):
		if player_id_to_country_id.has(player_id):
			var last_country_id: int = player_id_to_country_id[player_id]
			country_id_to_player_id.erase(last_country_id)
			unselect_nation.rpc(last_country_id)
		
		country_id_to_player_id[country_id] = player_id
		player_id_to_country_id[player_id] = country_id
		$click_noise.play()
		select_nation.rpc(country_id_to_tiles_owned[country_id])
		play_noise.rpc_id(player_id)

@rpc("any_peer", "call_local", "reliable")
func select_nation(cells_to_change: Array) -> void:
	var atlas: Vector2i = get_cell_atlas_coords(cells_to_change[0])
	for cell: Vector2i in cells_to_change:
		set_cell(cell, 1, atlas)

@rpc("authority", "call_local", "reliable")
func play_noise() -> void:
	$click_noise.play()

@rpc("any_peer", "call_local", "reliable")
func unselect_nation(cells_to_change: Array) -> void:
	var atlas: Vector2i = get_cell_atlas_coords(cells_to_change[0])
	for cell: Vector2i in cells_to_change:
		set_cell(cell, 0, atlas)

func is_owned(player_id: int, coords: Vector2i) -> bool:
	var toReturn: bool = false
	mutex.lock()
	if tile_to_country_id.has(coords):
		var country_id: int = tile_to_country_id[coords]
		if player_id > 0:
			#Is player
			toReturn = country_id_to_player_id.has(country_id) and country_id_to_player_id[country_id] == player_id
		else:
			#Is ai
			toReturn = ai_manager.get_instance().get_ai(player_id).country_id == country_id
	mutex.unlock()
	return toReturn

func get_owned_tiles(player_id: int) -> Array:
	var toReturn: Array = []
	mutex.lock()
	if player_id > 0 and player_id_to_country_id.has(player_id):
		toReturn = country_id_to_tiles_owned[player_id_to_country_id[player_id]]
	elif player_id < 0:
		var country_id: int =  ai_manager.get_instance().get_ai(player_id).country_id
		toReturn = country_id_to_tiles_owned[country_id]
	mutex.unlock()
	return toReturn

func get_player_id_from_cell(cell: Vector2i) -> int:
	var toReturn: int = -1 #TODO: -1 not accounted for so eventually need to be assert(false)
	mutex.lock()
	if tile_to_country_id.has(cell) and country_id_to_player_id.has(tile_to_country_id[cell]):
		toReturn = country_id_to_player_id[tile_to_country_id[cell]]
	mutex.unlock()
	return toReturn
	
