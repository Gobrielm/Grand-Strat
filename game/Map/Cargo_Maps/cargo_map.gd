extends TileMapLayer

@onready var cargo_values: Node = $cargo_values

var mutex: Mutex = Mutex.new()
var ais: Array[CompanyAi]

func _ready() -> void:
	Utils.assign_cargo_map(self)

func add_terminal_to_province(term: Terminal) -> void:
	var province_manager: ProvinceManager = ProvinceManager.get_instance()
	var prov: Province = province_manager.get_province(province_manager.get_province_id(term.get_location()))
	prov.add_terminal(term.get_location())

func remove_terminal_from_province(coords: Vector2i) -> void:
	var province_manager: ProvinceManager = ProvinceManager.get_instance()
	var prov: Province = province_manager.get_province(province_manager.get_province_id(coords))
	prov.remove_terminal(coords)

func transform_construction_site_to_factory(coords: Vector2i) -> void:
	TerminalMap.get_instance().transform_construction_site_to_factory(coords)
	set_tile.rpc(coords, Vector2i(4, 1))
	add_terminal_to_province(TerminalMap.get_instance().get_terminal(coords))

func place_random_industries() -> void:
	for prov: Province in ProvinceManager.get_instance().get_provinces():
		create_town_in_province(prov)
		
func create_town_in_province(prov: Province) -> void:
	var tile: Vector2i = prov.get_random_tile()
	if tile != Vector2i(0, 0): # Isn;t available
		create_town(tile, prov.get_province_id())

func create_town(coords: Vector2i, prov_id: int) -> void:
	const TOWN_THRESHOLD: int = 100000
	var province_manager: ProvinceManager = ProvinceManager.get_instance()
	if province_manager.get_population(prov_id) < TOWN_THRESHOLD:
		return
	var new_town: Town = Town.create(coords)
	Utils.world_map.make_cell_invisible(coords)
	set_tile.rpc(coords, Vector2i(0, 1))
	add_terminal_to_province(new_town)
	TerminalMap.get_instance().create_terminal(new_town)

func add_industries_to_towns() -> void:
	for country_id: int in tile_ownership.get_instance().get_country_ids():
		var ai: ProspectorAi = ProspectorAi.create(country_id, 1, CargoInfo.get_instance().get_cargo_type("grain"))
		ais.push_back(ai)
		var other: InitialBuilder = InitialBuilder.create(country_id)
		other.build_initital_factories()

func ai_cycle() -> void:
	for ai: ProspectorAi in ais:
		ai.month_tick()

func place_industry_for_town(town: Town, province: Province) -> void:
	var tile: Vector2i = town.get_location()
	var level: int = town.get_total_pops()
	if level > 100:
		place_random_group(tile, province, level, CargoInfo.get_instance().get_cargo_type("grain"))
		place_some_industries(tile, province, level)
	else:
		place_primary_near_middle(tile, level, CargoInfo.get_instance().get_cargo_type("grain"))

func place_random_group(tile: Vector2i, province: Province, town_level: int, type: int) -> void:
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
		place_road_depot(rand_tile, 0)
		for cell: Vector2i in get_surrounding_cells(rand_tile):
			if randi() % 2 == 0 and !Utils.is_tile_taken(cell):
				@warning_ignore("integer_division")
				place_primary_near_middle(cell, town_level, type)
		connect_road_depots(tile, rand_tile)

func connect_road_depots(tile1: Vector2i, tile2: Vector2i) -> void:
	RoadMap.get_instance().bfs_and_connect(tile1, tile2)

func place_primary_near_middle(middle: Vector2i, level: int, type: int) -> void:
	var tiles: Array = Utils.world_map.thread_get_surrounding_cells(middle)
	tiles.shuffle()
	var num: int = randi() % 2 + 1
	for tile: Vector2i in tiles:
		if !Utils.is_tile_taken(tile):
			@warning_ignore("integer_division")
			create_factory(0, tile, recipe.get_primary_recipe_for_type(type), level / num)
			num -= 1
			if num == 0:
				break

func place_some_industries(tile: Vector2i, province: Province, level: int) -> void:
	var cargo_info: CargoInfo = CargoInfo.get_instance()
	#var number_to_place: int = ceil((level - 100.0) / 100)
	var resources: Dictionary = get_most_prominent_resources(province)
	var factory_recipe: Array
	if (resources.has(cargo_info.get_cargo_type("salt")) and resources.has(cargo_info.get_cargo_type("grain"))):
		factory_recipe = recipe.get_secondary_recipe_for_types({cargo_info.get_cargo_type("salt"): true, cargo_info.get_cargo_type("grain"): true})
	elif (resources.has(cargo_info.get_cargo_type("wood"))):
		factory_recipe = recipe.get_secondary_recipe_for_types({cargo_info.get_cargo_type("wood"): true})
	elif (resources.has(cargo_info.get_cargo_type("cotton"))):
		factory_recipe = recipe.get_secondary_recipe_for_types({cargo_info.get_cargo_type("cotton"): true})
	
	if !factory_recipe.is_empty():
		var town: Town = TerminalMap.get_instance().get_town(tile)
		var fact: Factory = PrivateAiFactory.create(tile, factory_recipe[0], factory_recipe[1])
		town.add_factory(fact)

func place_random_road_depot(middle: Vector2i) -> Vector2i:
	var tiles: Array = Utils.world_map.thread_get_surrounding_cells(middle)
	tiles.shuffle()
	for tile: Vector2i in tiles:
		if !Utils.is_tile_taken(tile):
			place_road_depot(tile, 0)
			return tile
	return Vector2i(0, 0)

func place_road_depot(tile: Vector2i, owner_id: int) -> void:
	RoadMap.get_instance().place_road_depot(tile)
	var road_depot: RoadDepot = RoadDepot.new(tile, owner_id)
	add_terminal_to_province(road_depot)
	TerminalMap.get_instance().create_terminal(road_depot)

func get_most_prominent_resources(province: Province) -> Dictionary:
	var d: Dictionary = {}
	for tile: Vector2i in province.get_tiles():
		var resources: Dictionary = cargo_values.get_available_resources(tile)
		for type: int in resources:
			var amount: int = resources[type]
			if !d.has(type):
				d[type] = 0
			d[type] += amount
	#var toReturn: priority_queue = priority_queue.new()
	#for type: int in d:
		#toReturn.insert_element(type, d[type])
	
	return d

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
	create_factory(0, Vector2i(101, -117), recipe.get_primary_recipe_for_type(CargoInfo.get_instance().get_cargo_type("grain")), 1)
	place_random_road_depot(Vector2i(101, -117))
	create_town(Vector2i(101, -114), 177)
	place_random_road_depot(Vector2i(101, -114))
