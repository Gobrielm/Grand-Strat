extends RefCounted

var im_population: Image = preload("res://Map/Map_Info/population.png").get_image()
var population: TileMapLayer
var mutex: Mutex = Mutex.new()
var total: int = 0

func _init() -> void:
	pass

func create_population_map() -> void:
	population = preload("res://Map/Map_Info/population.tscn").instantiate()
	refresh_population()
	#print(total)
	#save_population()
	population.queue_free()

func helper(x: int, y: int, tile: Vector2i, map_data_obj: map_data) -> void:
	if Utils.is_tile_water(tile):
		return
	var color: Color = im_population.get_pixel(x, y)
	var num: int= 0
	var other_num: int = -1
	const multipler: float = 0.14
	if color.r > 0.15: #Non Cities
		if color.b > 0.9:  #0 Level / White
			@warning_ignore("narrowing_conversion")
			num = (randi() % 40000 + 10000) * multipler
			other_num = 1
		elif color.b > 0.70: #1 Level
			@warning_ignore("narrowing_conversion")
			num = (randi() % 50000 + 50000) * multipler
			other_num = 2
		elif color.b > 0.50: #2 Level
			@warning_ignore("narrowing_conversion")
			num = (randi() % 200000 + 100000) * multipler
			other_num = 3
		elif color.b > 0.30: #3 Level
			@warning_ignore("narrowing_conversion")
			num = (randi() % 200000 + 300000) * multipler
			other_num = 4
		elif color.b > 0.10: #4 Level
			@warning_ignore("narrowing_conversion")
			num = (randi() % 300000 + 500000) * multipler
			other_num = 5
	else:
		if color.g > 0.9: # 100K City
			num = (randi() % 100000 + 500000)
			other_num = 6
		elif color.b > 0.9: #1 Million City
			num = (randi() % 300000 + 1000000)
			other_num = 7
		else: #Black/Nothing
			@warning_ignore("narrowing_conversion")
			num = (randi() % 100) * multipler
			other_num = 0
	map_data_obj.add_population_to_province(tile, num)
	population.set_population(tile, other_num)
	mutex.lock()
	total += num
	mutex.unlock()


func save_population() -> void:
	var scene: PackedScene = PackedScene.new()
	var result: Error = scene.pack(population)
	if result == OK:
		var file: String = "res://Map/Map_Info/population.tscn"
		result = ResourceSaver.save(scene, file)
		if result != OK:
			push_error("An error occurred while saving the scene to disk.")


#Uses image to re-create population tilemaplayer, then remakes image to match tilemaplater
func refresh_population() -> void:
	var map_data_obj: map_data = map_data.get_instance()
	for real_x: int in range(-609, 671):
		for real_y: int in range(-243, 282):
			@warning_ignore("integer_division")
			var x: int = (real_x + 609) * 3 / 2
			@warning_ignore("integer_division")
			var y: int = (real_y + 243) * 7 / 4
			var tile: Vector2i = Vector2i(real_x, real_y)
			helper(x, y, tile, map_data_obj)

	var new_image: Image = Image.create(1920, 919, false, Image.FORMAT_RGBA8)
	for real_x: int in range(-609, 671):
		for real_y: int in range(-243, 282):
			@warning_ignore("integer_division")
			var x: int = (real_x + 609) * 3 / 2
			@warning_ignore("integer_division")
			var y: int = (real_y + 243) * 7 / 4
			var tile: Vector2i = Vector2i(real_x, real_y)
			if Utils.is_tile_water(tile):
				continue
			var num: int = population.get_population(tile)
			var color: Color = get_color_from_num(num)
			new_image.set_pixel(x, y, color)
			if x != 0 and y != 0:
				new_image.set_pixel(x - 1, y - 1, color)
			if y != 0:
				new_image.set_pixel(x, y - 1, color)
			if x != 0:
				new_image.set_pixel(x - 1, y, color)
	
	new_image.save_png("res://Map/Map_Info/population.png")

func get_color_from_num(num: int) -> Color:
	if num == 1:
		return Color(1, 1, 1)
	elif num == 2:
		return Color(1, 0.8, 0.8)
	elif num == 3:
		return Color(1, 0.6, 0.6)
	elif num == 4:
		return Color(1, 0.4, 0.4)
	elif num == 5:
		return Color(1, 0.2, 0.2)
	#Cities
	elif num == 6:
		return Color(0, 1, 0)
	elif num == 7:
		return Color(0, 0, 1)
	return Color(0, 0, 0)
