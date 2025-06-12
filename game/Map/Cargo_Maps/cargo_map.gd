extends TileMapLayer

@onready var cargo_values: Node = $cargo_values

var mutex: Mutex = Mutex.new()

func _ready() -> void:
	Utils.assign_cargo_map(self)

func add_terminal_to_province(term: Terminal) -> void:
	var map_dat: map_data = map_data.get_instance()
	var prov: Province = map_dat.get_province(map_dat.get_province_id(term.get_location()))
	prov.add_terminal(term.get_location())

func remove_terminal_from_province(coords: Vector2i) -> void:
	var map_dat: map_data = map_data.get_instance()
	var prov: Province = map_dat.get_province(map_dat.get_province_id(coords))
	prov.remove_terminal(coords)

func transform_construction_site_to_factory(coords: Vector2i) -> void:
	TerminalMap.get_instance().transform_construction_site_to_factory(coords)
	set_tile.rpc(coords, Vector2i(4, 1))
	add_terminal_to_province(TerminalMap.get_instance().get_terminal(coords))

func place_random_industries() -> void:
	var map_data_singleton: map_data = map_data.get_instance()
	for prov: Province in map_data_singleton.get_provinces():
		create_town_in_province(prov)
		
func create_town_in_province(prov: Province) -> void:
	var tile: Vector2i = prov.get_random_tile()
	if tile != Vector2i(0, 0): # Isn;t available
		create_town(tile, prov.get_province_id())

func create_town(coords: Vector2i, prov_id: int) -> void:
	const TOWN_THRESHOLD: int = 100000
	var map_dat: map_data = map_data.get_instance()
	if map_dat.get_population(prov_id) < TOWN_THRESHOLD:
		return
	var new_town: Town = Town.create(coords)
	Utils.world_map.make_cell_invisible(coords)
	set_tile.rpc(coords, Vector2i(0, 1))
	add_terminal_to_province(new_town)
	TerminalMap.get_instance().create_terminal(new_town)

func add_industries_to_towns() -> void:
	var threads: Array = []
	for province: Province in map_data.get_instance().get_provinces():
		if threads.size() > 10:
			threads.back().wait_to_finish()
			threads.pop_back()
		
		for tile: Vector2i in map_data.get_instance().get_province_terminal_tiles(province):
			var town: Town = TerminalMap.get_instance().get_town(tile)
			if town != null:
				var level: int = town.get_total_pops()
				if level > 100:
					var thread: Thread = Thread.new()
					thread.start(place_random_group.bind(tile, province, level))
					threads.push_back(thread)
				else:
					place_farms_near_town(tile, level)
				break
	for thread: Thread in threads:
		thread.wait_to_finish()

func place_random_group(tile: Vector2i, province: Province, town_level: int) -> void:
	tile = place_random_road_depot(tile)
	if tile == Vector2i(0, 0): 
		return
	var tries: int = 0
	var rand_tile: Vector2i
	while true:
		rand_tile = province.get_random_tile()
		if rand_tile.distance_to(tile) < 10 and rand_tile.distance_to(tile) > 2:
			break
		tries += 1
		if tries > 10:
			rand_tile = Vector2i(0, 0)
			break
	
	if rand_tile != Vector2i(0, 0):
		place_road_depot(rand_tile)
		for cell: Vector2i in get_surrounding_cells(rand_tile):
			if randi() % 2 == 0 and !Utils.is_tile_taken(cell):
				@warning_ignore("integer_division")
				place_random_industry(cell, town_level / 3)
		connect_road_depots(province, tile, rand_tile)

func connect_road_depots(province: Province, tile1: Vector2i, tile2: Vector2i) -> void:
	#var tiles: Array[Vector2i] = []
	#for tile: Vector2i in map_data.get_instance().get_province_terminal_tiles(province):
		#if tile != tile1 and TerminalMap.get_instance().is_road_depot(tile):
			#tiles.append(tile)
	#for tile: Vector2i in tiles:
	RoadMap.get_instance().bfs_and_connect(tile1, tile2)

func place_farms_near_town(middle: Vector2i, level: int) -> void:
	var tiles: Array = Utils.world_map.thread_get_surrounding_cells(middle)
	tiles.shuffle()
	var num: int = 0
	for tile: Vector2i in tiles:
		if !Utils.is_tile_taken(tile):
			@warning_ignore("integer_division")
			create_factory(0, tile, get_primary_recipe_for_type(CargoInfo.get_instance().get_cargo_type("grain")), level / 3)
			num += 1
			if num == 3:
				break

func place_random_road_depot(middle: Vector2i) -> Vector2i:
	var tiles: Array = Utils.world_map.thread_get_surrounding_cells(middle)
	tiles.shuffle()
	for tile: Vector2i in tiles:
		if !Utils.is_tile_taken(tile):
			place_road_depot(tile)
			return tile
	return Vector2i(0, 0)

func place_road_depot(tile: Vector2i) -> void:
	RoadMap.get_instance().place_road_depot(tile)
	var road_depot: RoadDepot = RoadDepot.new(tile, 0)
	add_terminal_to_province(road_depot)
	TerminalMap.get_instance().create_terminal(road_depot)

func place_random_industry(tile: Vector2i, mult: int) -> bool:
	#var best_resource: int = cargo_values.get_best_resource(tile)
	#if best_resource == -1:
		#return false
	var best_resource: int = 10 # Just place grain for now
	create_factory(0, tile, get_primary_recipe_for_type(best_resource), mult)
	return true

func get_primary_recipe_for_type(type: int) -> Array:
	for recipe_set: Array in recipe.get_set_recipes():
		for output: int in recipe_set[1]:
			if output == type:
				return recipe_set
	return []

func create_factory(p_player_id: int, coords: Vector2i, obj_recipe: Array, mult: int) -> void:
	var new_factory: Factory
	if p_player_id > 0:
		new_factory =  Factory.create(coords, p_player_id, obj_recipe[0], obj_recipe[1])
	elif p_player_id < 0:
		new_factory =  AiFactory.create(coords, p_player_id, obj_recipe[0], obj_recipe[1])
	else:
		new_factory = PrivateAiFactory.create(coords, obj_recipe[0], obj_recipe[1])
	for i: int in range(1, mult):
		new_factory.admin_upgrade()
	call_thread_safe("call_set_tile_rpc", coords, get_atlas_cell(obj_recipe))
	add_terminal_to_province(new_factory)
	TerminalMap.get_instance().create_terminal(new_factory)

func call_set_tile_rpc(coords: Vector2i, atlas_cell: Vector2i) -> void:
	set_tile.rpc(coords, atlas_cell)

func place_test_industry() -> void:
	var output: Dictionary = {}
	output[1] = 1
	create_factory(-1, Vector2i(0, 0), [{}, output], 1)

func get_atlas_cell(obj_recipe: Array) -> Vector2i:
	var output: Dictionary = obj_recipe[1]
	if obj_recipe[0].is_empty() and output.size() == 1:
		var primary_type: int = output.keys()[0]
		if (primary_type >= 2 and primary_type <= 7) or primary_type == 20:
			return Vector2i(3, 0)
		elif primary_type == 10 or primary_type == 13 or primary_type == 14 or (primary_type >= 16 and primary_type <= 19):
			return Vector2i(4, 0)
	return Vector2i(4, 1)

func create_construction_site(_player_id: int, coords: Vector2i) -> void:
	var new_factory: ConstructionSite = ConstructionSite.create(coords, _player_id)
	set_tile.rpc(coords, Vector2i(3, 1))
	add_terminal_to_province(new_factory)
	TerminalMap.get_instance().create_terminal(new_factory)

func get_available_primary_recipes(coords: Vector2i) -> Array:
	return cargo_values.get_available_primary_recipes(coords)

func place_resources(map: TileMapLayer) -> void:
	cargo_values.place_resources(map)

@rpc("authority", "call_local", "unreliable")
func set_tile(coords: Vector2i, atlas: Vector2i) -> void:
	mutex.lock()
	set_cell(coords, 0, atlas)
	mutex.unlock()
	Utils.world_map.make_cell_invisible(coords)

func _on_cargo_values_finished_created_map_resources() -> void:
	if !Utils.world_map.is_testing():
		place_random_industries()

func test() -> void:
	create_factory(0, Vector2i(101, -117), get_primary_recipe_for_type(CargoInfo.get_instance().get_cargo_type("grain")), 1)
	place_random_road_depot(Vector2i(101, -117))
	create_town(Vector2i(101, -114), 177)
	place_random_road_depot(Vector2i(101, -114))
