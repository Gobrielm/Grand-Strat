extends TileMapLayer

func add_tile_to_province(tile: Vector2i, id: int) -> void:
	var map_data_singleton: map_data = map_data.get_instance()
	map_data_singleton.create_new_if_empty(id)
	map_data_singleton.add_tile_to_province(id, tile)
	set_cell(tile, 0, get_atlas_for_id(id))

func get_atlas_for_id(id: int) -> Vector2i:
	var num: int = id % 32
	@warning_ignore("integer_division")
	return Vector2i(num % 8, num / 8)
