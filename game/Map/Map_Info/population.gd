extends TileMapLayer

var mutex: Mutex = Mutex.new()

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
	mutex.lock()
	set_cell(tile, 0, atlas)
	mutex.unlock()

func get_population(tile: Vector2i) -> int:
	var num: int = -1
	var atlas: Vector2i = get_cell_atlas_coords(tile)
	if atlas == Vector2i(3,1):
		num = 1
	elif atlas == Vector2i(4, 1):
		num = 2
	elif atlas == Vector2i(5, 1):
		num = 3
	elif atlas == Vector2i(6, 1):
		num = 4
	elif atlas == Vector2i(7, 1):
		num = 5
	# Cities
	elif atlas == Vector2i(2, 0):
		num = 6
	elif atlas == Vector2i(6, 2):
		num = 7
	return num
