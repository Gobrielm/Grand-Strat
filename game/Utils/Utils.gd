class_name Utils extends Node

static var cargo_map: TileMapLayer = null
static var cargo_values: Node = null
static var world_map: TileMapLayer = null
static var background_music: AudioStreamPlayer = null
static var unit_map: TileMapLayer = null

static var mutex: Mutex = Mutex.new()

static func round(num: float, places: int) -> float:
	return round(num * pow(10, places)) / pow(10, places)

static func assign_cargo_map(_cargo_map: Node) -> void:
	cargo_map = _cargo_map
	cargo_values = cargo_map.get_node("cargo_values")

static func assign_world_map(_world_map: TileMapLayer) -> void:
	world_map = _world_map

static func assign_background_music(_background_music: AudioStreamPlayer) -> void:
	background_music = _background_music

static func assign_unit_map(_unit_map: TileMapLayer) -> void:
	unit_map = _unit_map

static func is_tile_water(coords: Vector2i) -> bool:
	return world_map.is_water(coords)

static func is_tile_open(coords: Vector2i, id: int) -> bool:
	var val: bool = !terminal_map.get_instance().is_tile_taken(coords) and rail_placer.get_instance().get_track_connection_count(coords) == 0 and tile_ownership.get_instance().is_owned(id, coords)
	return val

static func just_has_rails(coords: Vector2i, id: int) -> bool:
		return !terminal_map.get_instance().is_tile_taken(coords) and tile_ownership.get_instance().is_owned(id, coords)

static func click_music() -> void:
	background_music.click()

static func is_music_playing() -> bool:
	return background_music.get_province()

static func turn_on_black_white_map() -> void:
	cargo_map.material.set_shader_parameter("onoff", 1)
	world_map.material.set_shader_parameter("onoff", 1)

static func turn_off_black_white_map() -> void:
	cargo_map.material.set_shader_parameter("onoff", 0)
	world_map.material.set_shader_parameter("onoff", 0)

static func is_ready() -> bool:
	return cargo_map != null and cargo_values != null and world_map != null and rail_placer != null and background_music != null
