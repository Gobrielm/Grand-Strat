extends TileMapLayer

var cw: int = 0
var taken_tiles: Dictionary[Vector2i, bool] = {}

const depth_one_side: int = 18

func _physics_process(_delta: float) -> void:
	clear_any_free_spaces()

func get_pixel_length_y() -> int:
	return depth_one_side * tile_set.tile_size.y

func set_cw(p_cw: int) -> void:
	cw = p_cw
	@warning_ignore("integer_division")
	var lower: int = cw / 2
	var upper: int = ceil(cw / 2.0)
	for x: int in range(-lower, upper):
		for y: int in range(-depth_one_side, depth_one_side + 1):
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

func is_flat(tile: Vector2i) -> bool:
	return get_cell_atlas_coords(tile).y == 0

func is_forested(tile: Vector2i) -> bool:
	return get_cell_atlas_coords(tile).y == 1

func is_hilly(tile: Vector2i) -> bool:
	return get_cell_atlas_coords(tile).y == 2

func is_mountainous(tile: Vector2i) -> bool:
	return get_cell_atlas_coords(tile).y == 3

func is_wet(tile: Vector2i) -> bool:
	return get_cell_atlas_coords(tile).y == 4

func is_invalid(tile: Vector2i) -> bool:
	return get_cell_atlas_coords(tile).y == -1

func get_next_open_cell(moving_up: bool) -> Variant:
	var y: int = depth_one_side if moving_up else -depth_one_side
	@warning_ignore("integer_division")
	for x: int in range(-cw / 2, ceil(cw / 2.0)):
		var tile: Vector2i = Vector2i(x, y)
		if !taken_tiles.has(tile):
			taken_tiles[tile] = true
			return tile
	return null

func clear_any_free_spaces() -> void:
	for tile: Vector2i in taken_tiles:
		if is_tile_free(tile):
			taken_tiles.erase(tile)

func is_tile_free(tile: Vector2i) -> bool:
	return !is_space_taken(Transform2D(0, map_to_local(tile)))

func is_space_taken(transformation: Transform2D) -> bool:
	var space_state: PhysicsDirectSpaceState2D = get_world_2d().direct_space_state
	var shape: Shape2D = $CollisionAreaChecker/CollisionShape2D.shape
	var ex_transform: Transform2D = transformation  # Includes rotation and position
	
	var arg: PhysicsShapeQueryParameters2D = PhysicsShapeQueryParameters2D.new()
	arg.shape = shape
	arg.transform = ex_transform
	arg.margin = 0.01
	var results: Array[Dictionary] = space_state.intersect_shape(arg, 10)
	
	for hit: Dictionary in results:
		if hit.collider is unit_icon:
			return true
	return false
