extends TileMapLayer

func add_tile_to_province(tile: Vector2i, id: int) -> void:
	set_cell(tile, 0, get_atlas_for_id(id))

func get_atlas_for_id(id: int) -> Vector2i:
	var num: int = id % 48
	@warning_ignore("integer_division")
	return Vector2i(num % 8, num / 8)
