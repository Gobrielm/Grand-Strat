extends RefCounted

var map: TileMapLayer
var mutex: Mutex = Mutex.new()

var im_volcanoes: Image = preload("res://Map/Map_Images/volcanos.png").get_image()
var im_lead: Image = preload("res://Map/Map_Images/lead.png").get_image()
var im_iron: Image = preload("res://Map/Map_Images/iron.png").get_image()
var im_coal: Image = preload("res://Map/Map_Images/coal.png").get_image()
var im_copper: Image = preload("res://Map/Map_Images/copper.png").get_image()
var im_zinc: Image = preload("res://Map/Map_Images/zinc.png").get_image()
var im_salt: Image = preload("res://Map/Map_Images/salt.png").get_image()
var im_cotton: Image = preload("res://Map/Map_Images/cotton.png").get_image()
var im_silk: Image = preload("res://Map/Map_Images/silk.png").get_image()
var im_spices: Image = preload("res://Map/Map_Images/spices.png").get_image()
var im_coffee: Image = preload("res://Map/Map_Images/coffee.png").get_image()
var im_tea: Image = preload("res://Map/Map_Images/tea.png").get_image()
var im_tobacco: Image = preload("res://Map/Map_Images/tobacco.png").get_image()
var im_gold: Image = preload("res://Map/Map_Images/gold.png").get_image()

func _init(_map: TileMapLayer) -> void:
	map = _map

func create_resource_array() -> Array:
	var toReturn: Array = []
	for i: int in CargoInfo.get_instance().amount_of_primary_goods:
		toReturn.push_back({})
	
	
	start_threads_on_grid(-609, 671, -243, 282, 4, toReturn)
	return toReturn

func start_threads_on_grid(x_min: int, x_max: int, y_min: int, y_max: int, n: int, toReturn: Array) -> void:
	assert(sqrt(n) == int(sqrt(n)), "n must be a perfect square (1, 4, 9, 16, ...)")

	var threads: Array[Thread] = []
	@warning_ignore("narrowing_conversion")
	var grid_size: int = (sqrt(n))
	@warning_ignore("integer_division")
	var x_step: int = ((x_max - x_min) / grid_size)
	@warning_ignore("integer_division")
	var y_step: int = ((y_max - y_min) / grid_size)

	for i: int in range(grid_size):
		for j: int in range(grid_size):
			var x_start: int = x_min + i * x_step
			var x_end: int = x_start + x_step
			var y_start: int = y_min + j * y_step
			var y_end: int = y_start + y_step
			
			if i == (grid_size - 1):
				x_end = 671
			if j == (grid_size - 1):
				y_end = 282
			
			var thread: Thread = Thread.new()
			thread.start(create_part_of_array.bind(x_start, x_end, y_start, y_end, toReturn))
			threads.append(thread)
	
	toReturn[0] = get_tiles_for_clay()
	
	for thread: Thread in threads:
		thread.wait_to_finish()

func get_tiles_for_clay() -> Dictionary:
	var toReturn: Dictionary = {}
	for tile: Vector2i in map.get_used_cells_by_id(0, Vector2i(6, 0)):
		if is_tile_river(tile):
			for cell: Vector2i in map.get_surrounding_cells(tile):
				if !is_tile_water(cell):
					mutex.lock()
					toReturn[cell] = 1
					mutex.unlock()
	for tile: Vector2i in map.get_used_cells_by_id(0, Vector2i(5, 0)):
		for cell: Vector2i in map.get_surrounding_cells(tile):
			if get_tile_elevation(map.get_cell_atlas_coords(cell)) == 0:
				mutex.lock()
				toReturn[cell] = 1
				mutex.unlock()
	
	return toReturn

func create_part_of_array(from_x: int, to_x: int, from_y: int, to_y: int, toReturn: Array) -> void:
	for real_x: int in range(from_x, to_x):
		for real_y: int in range(from_y, to_y):
			@warning_ignore("integer_division")
			var x: int = (real_x + 609) * 3 / 2
			@warning_ignore("integer_division")
			var y: int = (real_y + 243) * 7 / 4
			var tile: Vector2i = Vector2i(real_x, real_y)
			helper(x, y, tile, toReturn)

func helper(x: int, y: int, tile: Vector2i, toReturn: Array) -> void:
	
	add_basic_resource(toReturn, tile)
	
	var color: Color = im_volcanoes.get_pixel(x, y)
	if color.r > (color.b + color.g + 0.5) and !is_tile_water(tile):
		update_resource_array(2, tile, 1, toReturn)
	
	color = im_lead.get_pixel(x, y)
	if color.r > (color.b + color.g - 0.45) and color.r > 0.6 and !is_tile_water(tile):
		update_resource_array(3, tile, 1, toReturn)
	
	color = im_iron.get_pixel(x, y)
	if color.r > (color.b + color.g) or color.b > (color.r + color.g) and !is_tile_water(tile):
		update_resource_array(4, tile, 1, toReturn)
	
	color = im_coal.get_pixel(x, y)
	if color.r > 0.75 and color.r > (color.b + color.g - 0.3) and !is_tile_water(tile):
		update_resource_array(5, tile, 1, toReturn)
	
	color = im_copper.get_pixel(x, y)
	if color.b > (color.r + color.g - 0.4) and color.b > (color.r + 0.05) and !is_tile_water(tile):
		update_resource_array(6, tile, 1, toReturn)
	elif 0.01 < color.get_luminance() and color.get_luminance() < 0.65 and !is_tile_water(tile):
		update_resource_array(6, tile, 1, toReturn)
	
	color = im_zinc.get_pixel(x, y)
	if color.g > (color.r + color.b) and !is_tile_water(tile):
		update_resource_array(7, tile, 1, toReturn)
	
	color = im_salt.get_pixel(x, y)
	if color.r > 0.7 and !is_tile_water(tile):
		if color.r > 0.9:
			update_resource_array(9, tile, 3, toReturn)
		elif randi() % 5 == 0:
			update_resource_array(9, tile, 1, toReturn)
	
	color = im_cotton.get_pixel(x, y)
	if color.r > 0.75 and color.r > (color.b + 0.1) and !is_tile_water(tile):
		update_resource_array(14, tile, 1, toReturn)
	
	color = im_silk.get_pixel(x, y)
	if 1.3 < (color.b + color.g) and color.r > 0.75 and color.r > (color.b + 0.07) and !is_tile_water(tile):
		if randi() % 5 == 0:
			update_resource_array(15, tile, 1, toReturn)
	elif color.r > 0.75 and color.r > (color.b + 0.07) and !is_tile_water(tile):
		update_resource_array(15, tile, 1, toReturn)
	
	color = im_spices.get_pixel(x, y)
	if color.r > 0.75 and !is_tile_water(tile):
		update_resource_array(16, tile, 1, toReturn)
	
	color = im_coffee.get_pixel(x, y)
	if color.r > 0.9 and 0.9 > color.b and 0.9 > color.g and !is_tile_water(tile):
		if 0.7 > color.b and 0.7 > color.g:
			update_resource_array(17, tile, 1, toReturn)
		elif randi() % 5 == 0:
			update_resource_array(17, tile, 1, toReturn)
	
	color = im_tea.get_pixel(x, y)
	if !is_color_whitish(color) and !is_tile_water(tile):
		if 0.7 > color.b and 0.7 > color.g:
			update_resource_array(18, tile, 1, toReturn)
		elif randi() % 5 == 0:
			update_resource_array(18, tile, 1, toReturn)
	
	color = im_tobacco.get_pixel(x, y)
	if 1.3 < (color.b + color.g) and color.r > 0.75 and color.r > (color.b + 0.07) and !is_tile_water(tile):
		if randi() % 5 == 0:
			update_resource_array(19, tile, 1, toReturn)
	elif color.r > 0.75 and color.r > (color.b + 0.1) and !is_tile_water(tile):
		update_resource_array(19, tile, 1, toReturn)
	
	
	color = im_gold.get_pixel(x, y)
	if 1.3 < (color.b + color.g) and color.r > 0.75 and color.r > (color.b + 0.07) and !is_tile_water(tile):
		if randi() % 5 == 0:
			update_resource_array(20, tile, 1, toReturn)
	elif color.r > 0.75 and color.r > (color.b + 0.1) and !is_tile_water(tile):
		update_resource_array(20, tile, 1, toReturn)

func update_resource_array(type: int, tile: Vector2i, val: int, toReturn: Array) -> void:
	mutex.lock()
	toReturn[type][tile] = val
	mutex.unlock()

#Resource_array is array[good_index] -> dict
func add_basic_resource(resource_array: Array , tile: Vector2i) -> void:
	var atlas: Vector2i = map.get_cell_atlas_coords(tile)
	if is_forested(atlas):
		if atlas == Vector2i(1, 0):
			update_resource_array(13, tile, 1, resource_array)
		update_resource_array(8, tile, 1, resource_array)
		if is_dense_forest(atlas):
			update_resource_array(8, tile, 3, resource_array)
	elif is_plains(atlas) or atlas == Vector2i(3, 0):
		update_resource_array(10, tile, 1, resource_array)
		update_resource_array(11, tile, 1, resource_array)
		if is_lush_plains(atlas):
			update_resource_array(10, tile, 3, resource_array)
			update_resource_array(11, tile, 3, resource_array)
	
	if is_desert(atlas) and is_tile_within_4_tiles_of_water(tile):
		update_resource_array(1, tile, 1, resource_array)
	
	if is_coastal(tile):
		update_resource_array(12, tile, 1, resource_array)
		update_resource_array(1, tile, 2, resource_array)
	elif is_tile_near_water(tile):
		update_resource_array(1, tile, 2, resource_array)

func is_tile_near_water(coords: Vector2i) -> bool:
	if is_tile_water(coords):
		return false
	for tile: Vector2i in map.get_surrounding_cells(coords):
		if is_tile_water(tile):
			return true
	return false

func is_coastal(coords: Vector2i) -> bool:
	if is_tile_water(coords):
		return false
	for tile: Vector2i in map.get_surrounding_cells(coords):
		if is_tile_water(tile) and is_tile_surrounded_by_water(tile):
			return true
	return false

func is_tile_surrounded_by_water(coords: Vector2i) -> bool:
	var count: int = 0
	for tile: Vector2i in map.get_surrounding_cells(coords):
		if is_tile_water(tile):
			count += 1
	return count >= 3

func is_forested(atlas: Vector2i) -> bool:
	return ((atlas.y == 0 or atlas.y == 2) and (atlas.x == 1 or atlas.x == 2 or atlas.x == 4)) or atlas == Vector2i(4, 1)

func is_dense_forest(atlas: Vector2i) -> bool:
	return atlas.x == 2 and (atlas.y == 0 or atlas.y == 2)

func is_plains(atlas: Vector2i) -> bool:
	return (is_lush_plains(atlas) or atlas == Vector2i(6, 1))

func is_desert(atlas: Vector2i) -> bool:
	return atlas.y == 3

func is_tile_within_4_tiles_of_water(coords: Vector2i) -> bool:
	var visited: Dictionary = {}
	var queue: Array[Vector2i] = [coords]
	visited[coords] = 0
	while (!queue.is_empty()):
		var curr: Vector2i = queue.pop_front()
		if is_tile_water(curr):
			return true
		for tile: Vector2i in map.get_surrounding_cells(curr):
			if !visited.has(tile):
				visited[tile] = visited[curr] + 1
				if visited[tile] < 5:
					queue.push_back(tile)
	return false

func is_lush_plains(atlas: Vector2i) -> bool:
	return atlas == Vector2i(0, 0)

func is_tile_water(coords: Vector2i) -> bool:
	var atlas: Vector2i = map.get_cell_atlas_coords(coords)
	return atlas == Vector2i(6, 0) or atlas == Vector2i(7, 0)

func get_tile_elevation(atlas: Vector2i) -> int:
	if atlas == Vector2i(5, 0) or atlas == Vector2i(3, 3):
		return 2
	elif atlas == Vector2i(3, 0) or atlas == Vector2i(4, 0) or atlas == Vector2i(5, 1) or atlas == Vector2i(3, 2) or atlas == Vector2i(3, 3):
		return 1
	return 0

func is_tile_river(coords: Vector2i) -> bool:
	var count: int = 0
	for cell: Vector2i in map.get_surrounding_cells(coords):
		if !is_tile_water(cell):
			count += 1
	return count > 3

func is_color_whitish(color: Color) -> bool:
	return abs(color.r - color.b) < 0.1 and abs(color.g - color.b) < 0.1 and abs(color.r - color.g) < 0.1
