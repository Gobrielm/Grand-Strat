extends TileMapLayer

var colors: Array = [Color(1, 0, 0), Color(0, 1, 0), Color(0, 0, 1), Color(0.5, 0.5, 0), Color(0.5, 0, 0.5), Color(0, 0.5, 0.5)]

func add_tile_to_province(tile: Vector2i, id: int) -> void:
	var tile_info: Node = Utils.tile_info
	tile_info.create_new_if_empty(id)
	tile_info.add_tile_to_province(id, tile)
	set_cell(tile, 0, get_atlas_for_id(id))

func get_atlas_for_id(id: int) -> Vector2i:
	var num: int = id % 32
	@warning_ignore("integer_division")
	return Vector2i(num % 8, num / 8)

func get_color(tile: Vector2i) -> Color:
	var atlas: Vector2i = get_cell_atlas_coords(tile)
	var num: int = atlas.y * 8 + atlas.x
	if num < 0:
		return Color(0, 0, 0, 1)
	return colors[num]
