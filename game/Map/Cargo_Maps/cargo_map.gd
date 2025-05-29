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
	var mult: int = map_dat.get_population_as_level(prov_id)
	var country_id: int = tile_ownership.get_instance().get_country_id(coords)
	#TODO: Use country id to get unique ai to manage
	var new_town: terminal = town.new(coords)
	Utils.world_map.make_cell_invisible(coords)
	set_tile.rpc(coords, Vector2i(0, 1))
	add_terminal_to_province(new_town)
	terminal_map.create_terminal(new_town)

func add_terminal_to_province(term: terminal) -> void:
	var map_dat: map_data = map_data.get_instance()
	var prov: province = map_dat.get_province(map_dat.get_province_id(term.location))
	prov.add_terminal(term.location, term)

func remove_terminal_from_province(coords: Vector2i) -> void:
	var map_dat: map_data = map_data.get_instance()
	var prov: province = map_dat.get_province(map_dat.get_province_id(coords))
	prov.remove_terminal(coords)

func transform_construction_site_to_factory(coords: Vector2i) -> void:
	set_tile.rpc(coords, Vector2i(4, 1))
	add_terminal_to_province(terminal_map.get_terminal(coords))

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
		var prov: province = map_data_singleton.get_province(province_id)
		create_town_in_province(prov)
		for entry: Dictionary in chances:
			if pop > entry.threshold and randi() % entry.mod == 0:
				pick_and_place_random_industry(prov, entry.count, entry.mult)
				break
		
func create_town_in_province(prov: province) -> void:
	var tile: Vector2i = prov.get_random_tile()
	if tile != Vector2i(0, 0):
		create_town(tile, prov.province_id)

func pick_and_place_random_industry(prov: province, count: int, multiplier: int) -> void:
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
	#TODO: Give it to country company
	create_factory(-1, tile, get_primary_recipe_for_type(best_resource), mult)
	return true

func get_primary_recipe_for_type(type: int) -> Array:
	for recipe_set: Array in recipe.get_set_recipes():
		for output: int in recipe_set[1]:
			if output == type:
				return recipe_set
	return []

func create_factory(p_player_id: int, coords: Vector2i, obj_recipe: Array, mult: int) -> void:
	var new_factory: factory
	if p_player_id > 0:
		new_factory =  player_factory.new(coords, p_player_id, obj_recipe[0], obj_recipe[1])
	else:
		new_factory =  ai_factory.new(coords, p_player_id, obj_recipe[0], obj_recipe[1])
	for i: int in range(1, mult):
		new_factory.admin_upgrade()
	set_tile.rpc(coords, get_atlas_cell(obj_recipe))
	add_terminal_to_province(new_factory)
	terminal_map.create_terminal(new_factory)

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
	var new_factory: construction_site = construction_site.new(coords, _player_id)
	set_tile.rpc(coords, Vector2i(3, 1))
	add_terminal_to_province(new_factory)
	terminal_map.create_terminal(new_factory)

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
