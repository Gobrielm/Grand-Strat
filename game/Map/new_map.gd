extends TileMapLayer

var visited = {}

func _ready():
	var im: Image = Image.load_from_file("res://Map/Map_Images/map.png")
	var real_x = 0
	var real_y = 0
	for x in im.get_width():
		for y in im.get_height():
			if x % 3 == 0 or y % 7 == 0 or y % 7 == 1 or y % 7 == 2:
				continue
			var color: Color = im.get_pixel(x, y)
			var atlas_coords: Vector2i
			if is_mountainous(color.r, color.g, color.b, color.get_luminance()):
				atlas_coords = Vector2i(5, 0)
			elif is_hilly(color.r, color.g, color.b, color.get_luminance()):
				#Yellow
				atlas_coords = Vector2i(3, 0)
			elif color.g > 0.8:
				atlas_coords = Vector2i(0, 0)
			elif color.b > 0.88:
				atlas_coords = Vector2(6, 0)
			elif color.b > 0.8:
				atlas_coords = Vector2(7, 0)
			
			set_cell(Vector2i(real_x, real_y), 0, atlas_coords)
			real_y += 1
		if x % 3 != 0:
			real_x += 1
			real_y = 0
	double_parse()
	triple_parse()
	forest_parse()
	#save()

func save():
	var scene: PackedScene = PackedScene.new()
	var result = scene.pack(self)
	if result == OK:
		var file = "res://idk1.tscn"
		if FileAccess.file_exists(file):
			print("Tried to override existing file")
			assert(false)
		var error = ResourceSaver.save(scene, file)
		if error != OK:
			push_error("An error occurred while saving the scene to disk.")

func double_parse():
	for tile in get_used_cells_by_id(0, Vector2i(7, 0)):
		check_costal_waters(tile)
		convert_enclosed_deep_to_shallow(tile)
	
	for tile in get_used_cells_by_id(0, Vector2i(3, 0)):
		check_surrounding_moutains(tile)
	
	for tile in get_used_cells_by_id(0, Vector2i(5, 0)):
		check_surrounding_moutains(tile)
	
func triple_parse():
	visited.clear()
	for tile in get_used_cells_by_id(0, Vector2i(6, 0)):
		convert_surrounded_shallow_to_deep(tile)

func forest_parse():
	var im_forest: Image = Image.load_from_file("res://Map/Map_Images/forestry.png")
	var im_rain: Image = Image.load_from_file("res://Map/Map_Images/rainfall.png")
	var im_temp: Image = Image.load_from_file("res://Map/Map_Images/temp.png")
	var im_deserts: Image = Image.load_from_file("res://Map/Map_Images/deserts.png")
	var real_x = 0
	var real_y = 0
	for x in im_forest.get_width():
		for y in im_forest.get_height():
			if x % 3 == 0 or y % 7 == 0 or y % 7 == 1 or y % 7 == 2:
				continue
			
			var color: Color = im_forest.get_pixel(x, y)
			var tile: Vector2i = Vector2i(real_x, real_y)
			var atlas: Vector2i = get_cell_atlas_coords(tile)
			
			if color.g > 0.5:
				if atlas == Vector2i(0, 0):
					var randNum: int = randi() % 5
					if randNum == 0:
						set_cell(tile, 0, Vector2i(2, 0))
					elif randNum == 1 or randNum == 2 or randNum == 3:
						set_cell(tile, 0, Vector2i(1, 0))
				elif atlas == Vector2i(3, 0):
					set_cell(tile, 0, Vector2i(4, 0))
			rainfall_parse(im_rain.get_pixel(x, y), tile)
			temp_parse(im_temp.get_pixel(x, y), tile)
			desert_parse(im_deserts.get_pixel(x, y), tile)
			random_parse(tile)
			real_y += 1
		if x % 3 != 0:
			real_x += 1
			real_y = 0

func rainfall_parse(color: Color, coords: Vector2i):
	if color.r > (color.g + color.b):
		var atlas = get_cell_atlas_coords(coords)
		if atlas == Vector2i(0, 0):
			set_cell(coords, 0, Vector2i(6, 1))
		elif atlas == Vector2i(1, 0) or atlas == Vector2i(2, 0):
			set_cell(coords, 0, Vector2i(4, 1))
		elif atlas == Vector2i(3, 0):
			set_cell(coords, 0, Vector2i(5, 1))

func temp_parse(color: Color, coords: Vector2i):
	var atlas = get_cell_atlas_coords(coords)
	if color.r > 0.9:
		convert_tile_tundra(coords)

func desert_parse(color: Color, coords: Vector2i):
	if color.r > 0.8:
		convert_tile_desert(coords)

func random_parse(coords: Vector2i):
	var atlas = get_cell_atlas_coords(coords)
	if atlas == Vector2i(0, 0):
		if randi() % 12 == 0:
			set_cell(coords, 0, Vector2i(1, 0))
		elif randi() % 24 == 0:
			set_cell(coords, 0, Vector2i(6, 1))
		elif randi() % 24 == 0:
			set_cell(coords, 0, Vector2i(3, 0))
	elif atlas == Vector2i(6, 1):
		if randi() % 24 == 0:
			set_cell(coords, 0, Vector2i(4, 1))
		elif randi() % 48 == 0:
			set_cell(coords, 0, Vector2i(0, 0))
		elif randi() % 48 == 0:
			set_cell(coords, 0, Vector2i(5, 1))

func convert_tile_tundra(coords: Vector2i):
	var atlas = get_cell_atlas_coords(coords)
	if atlas.x >= 0 and atlas.x <= 4:
		set_cell(coords, 0, Vector2i(atlas.x, 2))
	elif atlas == Vector2i(6, 1):
		set_cell(coords, 0, Vector2i(0, 2))
	elif atlas == Vector2i(5, 1):
		set_cell(coords, 0, Vector2i(3, 2))
	elif atlas == Vector2i(4, 1):
		set_cell(coords, 0, Vector2i(1, 2))

func convert_tile_desert(coords: Vector2i):
	var atlas = get_cell_atlas_coords(coords)
	if atlas.y == 0:
		if atlas.x == 0:
			if randi() % 2 == 0:
				set_cell(coords, 0, Vector2i(0, 3))
			else:
				set_cell(coords, 0, Vector2i(2, 3))
		elif atlas.x == 3:
			set_cell(coords, 0, Vector2i(1, 3))
		elif atlas.x == 5:
			set_cell(coords, 0, Vector2i(3, 3))
	elif atlas == Vector2i(6, 1):
		if randi() % 2 == 0:
			set_cell(coords, 0, Vector2i(0, 3))
		else:
			set_cell(coords, 0, Vector2i(2, 3))
	elif atlas == Vector2i(5, 1):
		set_cell(coords, 0, Vector2i(1, 3))

func is_mountainous(r: float, g: float, b: float, l: float) -> bool:
	return two_floats_are_within_amount(g, b) and r > 0.2

func is_hilly(r: float, g: float, b: float, l: float) -> bool:
	if r > (g + b):
		return true
	elif r > (b + 0.2) and r > (g + 0.12):
		return randi() % 3 == 0
	return false

func two_floats_are_within_amount(num1: float, num2: float) -> bool:
	return abs(num1 - num2) < 0.03

func three_floats_are_within_amount(num1: float, num2: float, num3: float) -> bool:
	return abs(num1 - num2) < 0.03 and abs(num1 - num3) < 0.03 and abs(num2 - num3) < 0.03

func check_surrounding_moutains(coords: Vector2i):
	var count = 0
	for cell: Vector2i in get_surrounding_cells(coords):
		var atlas: Vector2i = get_cell_atlas_coords(cell)
		if atlas == Vector2i(3, 0) or atlas == Vector2i(5, 0):
			count += 1
		elif atlas == Vector2i(6, 0):
			set_cell(coords, 0, Vector2i(0, 0))
			return
	
	var placed = false
	if count > 4:
		while (!placed):
			for cell: Vector2i in get_surrounding_cells(coords):
				if randi() % 6 == 0:
					if randi() % 2 == 0:
						set_cell(cell, 0, Vector2(0, 0))
					else:
						set_cell(cell, 0, Vector2(3, 0))
					placed = true
					break

func check_costal_waters(coords: Vector2i):
	for cell: Vector2i in get_surrounding_cells(coords):
		var atlas: Vector2i = get_cell_atlas_coords(cell)
		if atlas == Vector2i(0, 0):
			set_cell(coords, 0, Vector2i(6, 0))


func convert_enclosed_deep_to_shallow(coords: Vector2i):
	if visited.has(coords):
		return
	var queue = [coords]
	var deep_tiles = {}
	visited[coords] = 1
	deep_tiles[coords] = 1
	while !queue.is_empty():
		var curr = queue.pop_front()
		for cell: Vector2i in get_surrounding_cells(curr):
			var atlas: Vector2i = get_cell_atlas_coords(cell)
			if atlas == Vector2i(7, 0) and !visited.has(cell):
				queue.push_back(cell)
				visited[cell] = 1
				deep_tiles[cell] = 1
	if deep_tiles.size() > 500:
		return
	for cell in deep_tiles:
		set_cell(cell, 0, Vector2i(6, 0))

func convert_surrounded_shallow_to_deep(coords: Vector2i):
	if visited.has(coords):
		return
	var queue = [coords]
	var deep_tiles = {}
	visited[coords] = 1
	deep_tiles[coords] = 1
	while !queue.is_empty():
		var curr = queue.pop_front()
		for cell: Vector2i in get_surrounding_cells(curr):
			var atlas: Vector2i = get_cell_atlas_coords(cell)
			if atlas == Vector2i(6, 0) and !visited.has(cell):
				queue.push_back(cell)
				visited[cell] = 1
				deep_tiles[cell] = 1
			elif atlas != Vector2i(7, 0) and atlas != Vector2i(6, 0):
				return
	if deep_tiles.size() > 500:
		return
	for cell in deep_tiles:
		set_cell(cell, 0, Vector2i(7, 0))
