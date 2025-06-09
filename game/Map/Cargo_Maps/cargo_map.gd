extends TileMapLayer

@onready var cargo_values: Node = $cargo_values

var mutex: Mutex = Mutex.new()

func _ready() -> void:
	Utils.assign_cargo_map(self)

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
	place_random_road_depot(coords)

func add_terminal_to_province(term: Terminal) -> void:
	var map_dat: map_data = map_data.get_instance()
	var prov: Province = map_dat.get_province(map_dat.get_province_id(term.get_location()))
	prov.add_terminal(term.get_location(), term)

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
	for province_id: int in map_data_singleton.provinces:
		var pop: int = map_data_singleton.get_population(province_id)
		var chances: Array = [
			{ "threshold": 10000000, "mod": 1, "mult": 5, "count": 3 },
			{ "threshold": 1000000,  "mod": 3, "mult": 4, "count": 2 },
			{ "threshold": 100000,   "mod": 5, "mult": 3, "count": 1 },
			{ "threshold": 10000,    "mod": 10, "mult": 2, "count": 1 },
			{ "threshold": 0,        "mod": 30, "mult": 1, "count": 1 }
		]
		var prov: Province = map_data_singleton.get_province(province_id)
		create_town_in_province(prov)
		for entry: Dictionary in chances:
			if pop > entry.threshold and randi() % entry.mod == 0:
				pick_and_place_random_industry(prov, entry.count, entry.mult)
				break
		
func create_town_in_province(prov: Province) -> void:
	var tile: Vector2i = prov.get_random_tile()
	if tile != Vector2i(0, 0):
		create_town(tile, prov.get_province_id())

func pick_and_place_random_industry(prov: Province, count: int, multiplier: int) -> void:
	for i: int in range(count):
		var result: bool = place_random_industry(prov.get_random_tile(), randi() % multiplier)
		var tries: int = 0
		while !result and tries < 10:
			result = place_random_industry(prov.get_random_tile(), randi() % multiplier)
			tries += 1

func place_random_industry(tile: Vector2i, mult: int) -> bool:
	var best_resource: int = cargo_values.get_best_resource(tile)
	if best_resource == -1:
		return false
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
	set_tile.rpc(coords, get_atlas_cell(obj_recipe))
	add_terminal_to_province(new_factory)
	TerminalMap.get_instance().create_terminal(new_factory)
	place_random_road_depot(coords)
	

func place_random_road_depot(middle: Vector2i) -> void:
	var tiles: Array = Utils.world_map.thread_get_surrounding_cells(middle)
	tiles.shuffle()
	for tile: Vector2i in tiles:
		if !Utils.is_tile_taken(tile):
			RoadMap.get_instance().place_road_depot(tile)
			var road_depot: RoadDepot = RoadDepot.new(tile, 0)
			add_terminal_to_province(road_depot)
			TerminalMap.get_instance().create_terminal(road_depot)
			return

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
	
	create_town(Vector2i(101, -114), 177)
