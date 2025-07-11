extends TileMapLayer

var cw: int = 0

const depth_one_side: int = 15

func set_cw(p_cw: int) -> void:
	cw = p_cw
	@warning_ignore("integer_division")
	var lower: int = cw / 2
	var upper: int = ceil(cw / 2.0)
	for x: int in range(-lower, upper):
		for y: int in range(-depth_one_side, depth_one_side):
			set_cell(Vector2i(x, y), 0, Vector2i(0, 0))
