extends RefCounted

var im_population: Image = preload("res://Map/Map_Images/population.png").get_image()
var population: TileMapLayer
var mutex: Mutex = Mutex.new()
var total: int = 0

func _init() -> void:
	pass

func create_population_map() -> void:
	population = preload("res://Map/Map_Info/population.tscn").instantiate()
	var map_data_obj: map_data = map_data.get_instance()
	assert(map_data_obj != null)
	var thread: Thread = Thread.new()
	var thread1: Thread = Thread.new()
	var thread2: Thread = Thread.new()
	var thread3: Thread = Thread.new()
	thread.start(create_part_of_array.bind(-609, 0, -243, 0, map_data_obj))
	thread1.start(create_part_of_array.bind(0, 671, -243, 0, map_data_obj))
	thread2.start(create_part_of_array.bind(-609, 0, 0, 282, map_data_obj))
	thread3.start(create_part_of_array.bind(0, 671, 0, 282, map_data_obj))
	var threads: Array = [thread, thread1, thread2, thread3]
	for thd: Thread in threads:
		thd.wait_to_finish()
	#save_population()
	#print(total)
	population.queue_free()

func create_part_of_array(from_x: int, to_x: int, from_y: int, to_y: int, map_data_obj: map_data) -> void:
	for real_x: int in range(from_x, to_x):
		for real_y: int in range(from_y, to_y):
			@warning_ignore("integer_division")
			var x: int = (real_x + 609) * 3 / 2
			@warning_ignore("integer_division")
			var y: int = (real_y + 243) * 7 / 4
			var tile: Vector2i = Vector2i(real_x, real_y)
			helper(x, y, tile, map_data_obj)

func helper(x: int, y: int, tile: Vector2i, map_data_obj: map_data) -> void:
	if Utils.is_tile_water(tile):
		return
	var color: Color = im_population.get_pixel(x, y)
	var num: int= 0
	var other_num: int = -1
	const multipler: float = 0.2
	if color.r > 0.9:
		if color.b > 0.98:
			@warning_ignore("narrowing_conversion")
			num = (randi() % 100) * multipler
			other_num = 0
		elif color.b > 0.7:
			@warning_ignore("narrowing_conversion")
			num = (randi() % 40000 + 10000) * multipler
			other_num = 1
		elif color.b > 0.60:
			@warning_ignore("narrowing_conversion")
			num = (randi() % 50000 + 50000) * multipler
			other_num = 2
		elif color.b > 0.30:
			@warning_ignore("narrowing_conversion")
			num = (randi() % 200000 + 100000) * multipler
			other_num = 3
		elif color.b > 0.20:
			@warning_ignore("narrowing_conversion")
			num = (randi() % 200000 + 300000) * multipler
			other_num = 4
		elif color.b == 0.0:
			@warning_ignore("narrowing_conversion")
			num = (randi() % 300000 + 500000) * multipler
			other_num = 5
	else:
		if color.b > 0.5:
			num = (randi() % 100000 + 500000)
			other_num = 6
		elif color.b > 0.25:
			num = (randi() % 300000 + 1000000)
			other_num = 7
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
