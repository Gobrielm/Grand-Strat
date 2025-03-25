extends TileMapLayer


# Called when the node enters the scene tree for the first time.
func _ready():
	for tile in get_used_cells_by_id(0, Vector2i(6, 0)):
		check_water_tiles(tile)
	save()

func save():
	var scene: PackedScene = PackedScene.new()
	var result = scene.pack(self)
	if result == OK:
		var file = "res://Map/Main_maps/world_map.tscn"
		if FileAccess.file_exists(file):
			print("Overriding Existing File")
		var error = ResourceSaver.save(scene, file)
		if error != OK:
			push_error("An error occurred while saving the scene to disk.")

func check_water_tiles(coords: Vector2i):
	for tile in get_surrounding_cells(coords):
		if is_desert(tile) and randi() % 2 == 0:
			convert_desert_to_plains(tile)
		elif is_plains(tile) and randi() % 2 == 0:
			convert_plains_to_grasslands(tile)
			

func is_desert(coords: Vector2i) -> bool:
	return get_cell_atlas_coords(coords).y == 3

func is_plains(coords: Vector2i) -> bool:
	var atlas = get_cell_atlas_coords(coords)
	return atlas.y == 1 and atlas.x >= 4 and atlas.x <= 6

func convert_desert_to_plains(coords: Vector2i):
	var atlas = get_cell_atlas_coords(coords)
	if atlas.x == 0 or atlas.x == 2:
		if randi() % 4 != 0:
			set_cell(coords, 0, Vector2i(6, 1))
		else:
			set_cell(coords, 0, Vector2i(4, 1))
	elif atlas.x == 1:
		set_cell(coords, 0, Vector2i(5, 1))
	elif atlas.x == 3:
		set_cell(coords, 0, Vector2i(5, 0))

func convert_plains_to_grasslands(coords: Vector2i):
	var atlas = get_cell_atlas_coords(coords)
	if atlas.x == 5:
		set_cell(coords, 0, Vector2i(3, 0))
	elif atlas.x == 6:
		if randi() % 4 != 0:
			set_cell(coords, 0, Vector2i(0, 0))
		else:
			set_cell(coords, 0, Vector2i(1, 0))
	elif atlas.x == 4:
		set_cell(coords, 0, Vector2i(1, 0))
	
