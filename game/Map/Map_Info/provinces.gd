extends TileMapLayer

var colors = [Color(1, 0, 0), Color(0, 1, 0), Color(0, 0, 1), Color(0.5, 0.5, 0), Color(0.5, 0, 0.5), Color(0, 0.5, 0.5)]

func add_tile_to_province(tile: Vector2i, id: int):
	var tile_info = Utils.tile_info
	tile_info.create_new_if_empty(id)
	tile_info.add_tile_to_province(id, tile)
	set_cell(tile, 0, get_atlas_for_id(id))

func get_atlas_for_id(id: int) -> Vector2i:
	var num = id % 32
	@warning_ignore("integer_division")
	return Vector2i(num % 8, num / 8)

func get_color(tile: Vector2i) -> Color:
	var atlas = get_cell_atlas_coords(tile)
	var num = atlas.y * 8 + atlas.x
	if num < 0:
		return Color(0, 0, 0, 1)
	return colors[num]
