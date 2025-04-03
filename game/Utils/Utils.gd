class_name Utils extends Node

static var cargo_map: TileMapLayer
static var cargo_values: Node
static var tile_ownership: TileMapLayer
static var world_map: TileMapLayer
static var rail_placer: Node
static var background_music: AudioStreamPlayer

static func round(num: float, places: int) -> float:
	return round(num * pow(10, places)) / pow(10, places)

static func assign_cargo_map(_cargo_map: Node) -> void:
	cargo_map = _cargo_map
	cargo_values = cargo_map.cargo_values

static func assign_tile_ownership(_tile_ownership: TileMapLayer) -> void:
	tile_ownership = _tile_ownership

static func assign_rail_placer(_rail_placer: Node) -> void:
	rail_placer = _rail_placer

static func assign_world_map(_world_map: TileMapLayer) -> void:
	world_map = _world_map

static func assign_background_music(_background_music: AudioStreamPlayer) -> void:
	background_music = _background_music

static func is_tile_water(coords: Vector2i) -> bool:
	var atlas: Vector2i = world_map.get_cell_atlas_coords(coords)
	return atlas == Vector2i(6, 0) or atlas == Vector2i(7, 0) or atlas == Vector2i(-1, -1)

static func is_tile_open(coords: Vector2i, id: int) -> bool:
	return !terminal_map.is_tile_taken(coords) and rail_placer.get_track_connection_count(coords) == 0 and tile_ownership.is_owned(id, coords)

static func just_has_rails(coords: Vector2i, id: int) -> bool:
		return !terminal_map.is_tile_taken(coords) and tile_ownership.is_owned(id, coords)

static func click_music() -> void:
	background_music.click()

static func is_music_playing() -> bool:
	return background_music.get_state()

static func turn_on_black_white_map() -> void:
	cargo_map.material.set_shader_parameter("onoff", 1)
	world_map.material.set_shader_parameter("onoff", 1)

static func turn_off_black_white_map() -> void:
	cargo_map.material.set_shader_parameter("onoff", 0)
	world_map.material.set_shader_parameter("onoff", 0)
