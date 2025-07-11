class_name unit_locations extends TileMapLayer

var unit_info: Dictionary[Vector2i, base_unit] = {}

var cw: int = 0
const depth_one_side: int = 15

func erase_unit(tile: Vector2i) -> void:
	erase_cell(tile)

func set_unit(tile: Vector2i) -> void:
	set_cell(tile, 0, Vector2i(0, 1))

func is_valid(tile: Vector2i) -> bool:
	return get_cell_atlas_coords(tile) == Vector2i(0, 0)

func set_cw(p_cw: int) -> void:
	cw = p_cw
	#@warning_ignore("integer_division")
	#var lower: int = cw / 2
	#var upper: int = ceil(cw / 2.0)
	#for x: int in range(-lower, upper):
		#for y: int in range(-depth_one_side, depth_one_side + 1):
			#erase_unit(Vector2i(x, y))

# Defenders are -y, top
func deploy_def(unit: base_unit) -> void:
	@warning_ignore("integer_division")
	var base_tile: Vector2i = Vector2i(-cw / 2, -depth_one_side)
	var tile: Vector2i = base_tile
	while unit_info.has(tile):
		tile.x += 1
		@warning_ignore("integer_division")
		if tile.x == cw / 2: # if reach end of path then move down and to the left
			tile.x *= -1
			tile.y += 1
			if tile.y == 0:
				return # Cannot deploy past middle
		
	unit_info[tile] = unit
	set_cell(tile, 0, unit.get_atlas_coord())

# Attackers are +y, bottom
func deploy_atk(unit: base_unit) -> void:
	@warning_ignore("integer_division")
	var base_tile: Vector2i = Vector2i(-cw / 2, depth_one_side)
	var tile: Vector2i = base_tile
	while unit_info.has(tile):
		tile.x += 1
		@warning_ignore("integer_division")
		if tile.x == cw / 2: # if reach end of path then move up and to the left
			tile.x *= -1
			tile.y -= 1
			if tile.y == 0:
				return # Cannot deploy past middle
		
	unit_info[tile] = unit
	set_cell(tile, 0, unit.get_atlas_coord())
