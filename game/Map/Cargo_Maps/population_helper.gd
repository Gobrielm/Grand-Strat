extends Node

var im_population: Image = preload("res://Map/Map_Images/population.png").get_image()
var population: TileMapLayer
var mutex := Mutex.new()
var total := 0

func _init():
	pass

func create_population_map():
	population = preload("res://Map/Map_Info/population.tscn").instantiate()
	var tile_info = Utils.tile_info
	assert(tile_info != null)
	var thread := Thread.new()
	var thread1 := Thread.new()
	var thread2 := Thread.new()
	var thread3 := Thread.new()
	thread.start(create_part_of_array.bind(-609, 0, -243, 0, tile_info))
	thread1.start(create_part_of_array.bind(0, 671, -243, 0, tile_info))
	thread2.start(create_part_of_array.bind(-609, 0, 0, 282, tile_info))
	thread3.start(create_part_of_array.bind(0, 671, 0, 282, tile_info))
	var threads := [thread, thread1, thread2, thread3]
	for thd: Thread in threads:
		thd.wait_to_finish()
	#save_population()
	#print(total)
	population.queue_free()

func create_part_of_array(from_x: int, to_x: int, from_y: int, to_y: int, tile_info):
	for real_x in range(from_x, to_x):
		for real_y in range(from_y, to_y):
			var x := (real_x + 609) * 3 / 2
			var y := (real_y + 243) * 7 / 4
			var tile := Vector2i(real_x, real_y)
			helper(x, y, tile, tile_info)

func helper(x: int, y: int, tile: Vector2i, tile_info):
	if Utils.is_tile_water(tile):
		return 0
	var color: Color = im_population.get_pixel(x, y)
	var num := 0
	var other_num := -1
	var multipler := 0.2
	if color.r > 0.9:
		if color.b > 0.98:
			num = (randi() % 100) * multipler
			other_num = 0
		elif color.b > 0.7:
			num = (randi() % 40000 + 10000) * multipler
			other_num = 1
		elif color.b > 0.60:
			num = (randi() % 50000 + 50000) * multipler
			other_num = 2
		elif color.b > 0.30:
			num = (randi() % 200000 + 100000) * multipler
			other_num = 3
		elif color.b > 0.20:
			num = (randi() % 200000 + 300000) * multipler
			other_num = 4
		elif color.b == 0.0:
			num = (randi() % 300000 + 500000) * multipler
			other_num = 5
	else:
		if color.b > 0.5:
			num = (randi() % 100000 + 500000)
			other_num = 6
		elif color.b > 0.25:
			num = (randi() % 300000 + 1000000)
			other_num = 7
	mutex.lock()
	tile_info.add_population_to_province(tile, num)
	population.set_population(tile, other_num)
	total += num
	mutex.unlock()
	

func save_population():
	var scene := PackedScene.new()
	var result = scene.pack(population)
	if result == OK:
		var file = "res://Map/Map_Info/population.tscn"
		var error = ResourceSaver.save(scene, file)
		if error != OK:
			push_error("An error occurred while saving the scene to disk.")
