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
			place_tile(x, y)

func place_tile(x: int, y: int) -> void:
	var num: int = randi() % 10
	if num < 4:
		place_plains(x, y)
	elif num < 6:
		place_forest(x, y)
	elif num < 8:
		place_hill(x, y)
	else:
		place_mountain(x, y)

func place_plains(x: int, y: int) -> void:
	set_cell(Vector2i(x, y), 0, Vector2i(randi() % 3, 0))

func place_forest(x: int, y: int) -> void:
	set_cell(Vector2i(x, y), 0, Vector2i(randi() % 3, 1))

func place_hill(x: int, y: int) -> void:
	set_cell(Vector2i(x, y), 0, Vector2i(randi() % 3, 2))

func place_mountain(x: int, y: int) -> void:
	set_cell(Vector2i(x, y), 0, Vector2i(randi() % 3, 3))
