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
	var town_tiles_to_place: Array[Vector2i] = []
	for prov: Province in ProvinceManager.get_instance().get_provinces():
		var tile: Variant = create_town_in_province(prov)
		if tile:
			town_tiles_to_place.append(tile as Vector2i)
	call_deferred("place_towns_client_side", town_tiles_to_place)
	
	
func create_town_in_province(prov: Province) -> Variant:
	var tile: Vector2i = prov.get_random_tile()
	if tile != Vector2i(0, 0): # Isn;t available
		if create_town(tile, prov.get_province_id()):
			return tile
	return null

## Returns true if town sucessfully placed
func create_town(coords: Vector2i, prov_id: int) -> bool:
	const TOWN_THRESHOLD: int = 100000
	var province_manager: ProvinceManager = ProvinceManager.get_instance()
	if province_manager.get_population(prov_id) < TOWN_THRESHOLD:
		return false
	FactoryCreator.get_instance().create_town(coords)
	Utils.world_map.make_cell_invisible(coords)
	return true

func place_towns_client_side(town_tiles: Array[Vector2i]) -> void:
	for tile: Vector2i in town_tiles:
		set_tile.rpc(tile, Vector2i(0, 1))

func place_factories_client_side(fact_tiles: Dictionary) -> void:
	mutex.lock()
	
	for tile: Vector2i in fact_tiles:
		var atlas: Vector2i = get_atlas_cell(fact_tiles[tile])
		set_cell(tile, 0, atlas)
		Utils.world_map.make_cell_invisible(tile)
	
	mutex.unlock()

func add_industries_to_towns() -> void:
	for country_id: int in tile_ownership.get_instance().get_country_ids():
		var ai: ProspectorAi = ProspectorAi.create(country_id, 1, CargoInfo.get_instance().get_cargo_type("grain"))
		ais.push_back(ai)
		var other: InitialBuilder = InitialBuilder.create(country_id)
		other.build_initital_factories()

func ai_cycle() -> void:
	for ai: ProspectorAi in ais:
		ai.month_tick()

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
	FactoryCreator.get_instance().create_road_depot(tile, owner_id)

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
	var new_factory: Factory = TerminalMap.get_instance().create_factory(coords, p_player_id, obj_recipe[0], obj_recipe[1])
	
	for i: int in range(1, mult):
		new_factory.admin_upgrade()
	call_thread_safe("call_set_tile_rpc", coords, (obj_recipe[1] as Dictionary).keys()[0])
	add_terminal_to_province(new_factory)
	TerminalMap.get_instance().create_terminal(new_factory)

func call_set_tile_rpc(coords: Vector2i, type: int) -> void:
	set_tile.rpc(coords, get_atlas_cell(type))

func get_atlas_cell(primary_type: int = -1) -> Vector2i:
	if primary_type != -1:
		if (primary_type >= 2 and primary_type <= 7) or primary_type == 20:
			return Vector2i(3, 0)
		elif primary_type == 10 or primary_type == 13 or primary_type == 14 or (primary_type >= 16 and primary_type <= 19):
			return Vector2i(4, 0)
		elif primary_type == 8:
			return Vector2i(1, 0)
	return Vector2i(4, 1)

func create_construction_site(_player_id: int, coords: Vector2i) -> void:
	FactoryCreator.get_instance().create_construction_site(coords, _player_id)
	place_construction_site_tile(coords)

func place_construction_site_tile(coords: Vector2i) -> void:
	set_tile.rpc(coords, Vector2i(3, 1))

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
	FactoryCreator.get_instance().create_primary_industry(10, Vector2i(101, -117), 0, 1)
	var tile1: Vector2i = place_random_road_depot(Vector2i(101, -117))
	
	FactoryCreator.get_instance().create_primary_industry(10, Vector2i(101, -113), 0, 1)
	create_town(Vector2i(101, -114), 177)
	var tile2: Vector2i = place_random_road_depot(Vector2i(101, -114))
	RoadMap.get_instance().bfs_and_connect(tile1, tile2)
