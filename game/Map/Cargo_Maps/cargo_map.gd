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
	set_cell(coords, 0, Vector2i(4, 1))

func place_random_industry(tile: Vector2i) -> void:
	var tile_ownership: Node = Utils.tile_ownership
	create_factory(tile_ownership.get_player_id_from_cell(tile), tile)

func create_factory(_player_id: int, coords: Vector2i) -> void:
	var new_factory: construction_site = load("res://Cargo/Cargo_Objects/Specific/Player/construction_site.gd").new(coords, _player_id)
	set_cell(coords, 0, Vector2i(3, 1))
	terminal_map.create_terminal(new_factory)

func create_town(coords: Vector2i) -> void:
	var tile_ownership: Node = Utils.tile_ownership
	var new_town: terminal = load("res://Cargo/Cargo_Objects/Specific/Endpoint/town.gd").new(coords, tile_ownership.get_player_id_from_cell(coords))
	set_cell(coords, 0, Vector2i(0, 1))
	terminal_map.create_terminal(new_town)

	
func get_available_primary_recipes(coords: Vector2i) -> Array:
	return cargo_values.get_available_primary_recipes(coords)

func place_resources(map: TileMapLayer) -> void:
	cargo_values.place_resources(map)
