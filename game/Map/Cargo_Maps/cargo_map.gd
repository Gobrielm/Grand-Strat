extends TileMapLayer

@onready var cargo_values: Node = $cargo_values

func _ready() -> void:
	Utils.assign_cargo_map(self)
	for cell: Vector2i in get_used_cells():
		var building: terminal = instance_preplaced_towns(cell)
		terminal_map.create_terminal(building)

func instance_preplaced_towns(coords: Vector2i) -> terminal:
	var atlas: Vector2i = get_cell_atlas_coords(coords)
	#No other types accounted for, just towns should be preplaced. TODO: Towns automated too.
	assert(atlas == Vector2i(0, 1))
	var loaded_script: Resource = load("res://Cargo/Cargo_Objects/Specific/Endpoint/town.gd")
	assert(loaded_script != null)
	return loaded_script.new(coords, Utils.tile_ownership.get_player_id_from_cell(coords))

func transform_construction_site_to_factory(coords: Vector2i) -> void:
	set_tile(coords, Vector2i(4, 1))

func place_random_industries() -> void:
	var map_data_singleton: map_data = map_data.get_instance()
	for province_id: int in map_data_singleton.provinces:
		var pop: int = map_data_singleton.get_population(province_id)
		var chances: Array = [
			{ "threshold": 10000000, "mod": 1 },
			{ "threshold": 1000000,  "mod": 3 },
			{ "threshold": 100000,   "mod": 5 },
			{ "threshold": 10000,    "mod": 10 },
			{ "threshold": 0,        "mod": 30 }
		]
		for entry: Dictionary in chances:
			if pop > entry.threshold and randi() % entry.mod == 0:
				pick_and_place_random_industry(map_data_singleton, province_id)
				break


func pick_and_place_random_industry(map_data_singleton: map_data, province_id: int) -> void:
	var prov: province = map_data_singleton.get_province(province_id)
	place_random_industry(prov.get_random_tile())

func place_random_industry(tile: Vector2i) -> void:
	var tile_ownership: Node = Utils.tile_ownership
	var best_resource: int = cargo_values.get_best_resource(tile)
	if best_resource == -1:
		return
	create_factory(tile_ownership.get_player_id_from_cell(tile), tile, get_primary_recipe_for_type(best_resource))

func get_primary_recipe_for_type(type: int) -> Array:
	for recipe_set: Array in recipe.get_set_recipes():
		for output: int in recipe_set[1]:
			if output == type:
				return recipe_set
	return []

func create_factory(p_player_id: int, coords: Vector2i, obj_recipe: Array) -> void:
	var new_factory: factory
	if p_player_id > 0:
		new_factory =  player_factory.new(coords, p_player_id, obj_recipe[0], obj_recipe[1])
	else:
		new_factory =  ai_factory.new(coords, p_player_id, obj_recipe[0], obj_recipe[1])
	set_tile(coords, get_atlas_cell(obj_recipe))
	terminal_map.create_terminal(new_factory)



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
	var new_factory: construction_site = load("res://Cargo/Cargo_Objects/Specific/Player/construction_site.gd").new(coords, _player_id)
	set_tile(coords, Vector2i(3, 1))
	terminal_map.create_terminal(new_factory)

func create_town(coords: Vector2i) -> void:
	var tile_ownership: Node = Utils.tile_ownership
	var new_town: terminal = load("res://Cargo/Cargo_Objects/Specific/Endpoint/town.gd").new(coords, tile_ownership.get_player_id_from_cell(coords))
	set_tile(coords, Vector2i(0, 1))
	terminal_map.create_terminal(new_town)

func get_available_primary_recipes(coords: Vector2i) -> Array:
	return cargo_values.get_available_primary_recipes(coords)

func place_resources(map: TileMapLayer) -> void:
	cargo_values.place_resources(map)

func set_tile(coords: Vector2i, atlas: Vector2i) -> void:
	set_cell(coords, 0, atlas)
	Utils.world_map.make_cell_invisible(coords)

func _on_cargo_values_finished_created_map_resources() -> void:
	place_random_industries()
