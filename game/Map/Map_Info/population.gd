extends TileMapLayer

func set_population(tile: Vector2i, num: int) -> void:
	var atlas: Vector2i
	if num == 1:
		atlas = Vector2i(3,1)
	elif num == 2:
		atlas = Vector2i(4,1)
	elif num == 3:
		atlas = Vector2i(5,1)
	elif num == 4:
		atlas = Vector2i(6,1)
	elif num == 5:
		atlas = Vector2i(7,1)
	#Cities
	elif num == 6:
		atlas = Vector2i(2,0)
	elif num == 7:
		atlas = Vector2i(6,2)
	else:
		atlas = Vector2i(1, 3)
	set_cell(tile, 0, atlas)
