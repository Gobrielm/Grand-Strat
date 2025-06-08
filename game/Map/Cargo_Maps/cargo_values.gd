extends Node2D

var map: TileMapLayer

const TILES_PER_ROW: int = 8
const MAX_RESOURCES: Array = [5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, -1, 5000, -1, -1, -1, -1, 5000, 5000, 5000, 5000, 5000, 50000, 1000]
var magnitude_layers: Array = []
var mutex: Mutex = Mutex.new()

signal finished_created_map_resources

func _ready() -> void:
	create_magnitude_layers()

func can_build_type(type: int, coords: Vector2i) -> bool:
	return get_tile_magnitude(coords, type) > 0

func create_magnitude_layers() -> void:
	for child: Node in get_children():
		if child is TileMapLayer:
			magnitude_layers.append(child)

func get_layer(type: int) -> TileMapLayer:
	mutex.lock()
	var layer: TileMapLayer = magnitude_layers[type]
	mutex.unlock()
	assert(layer != null and layer.name == ("Layer" + str(type) + get_good_name_uppercase(type)))
	return layer

func set_resource_rpc(type: int, coords: Vector2i, atlas: Vector2i) -> void:
	set_cell_rpc.rpc(type, coords, atlas)

func set_resource_locally(type: int, coords: Vector2i, atlas: Vector2i) -> void:
	get_layer(type).set_cell(coords, 1, atlas)

@rpc("authority", "call_remote", "reliable")
func set_cell_rpc(type: int, coords: Vector2i, atlas: Vector2i) -> void:
	get_layer(type).set_cell(coords, 1, atlas)

func get_layers() -> Array:
	return magnitude_layers

func get_best_resource(coords: Vector2i) -> int:
	var resources: Dictionary = get_available_resources(coords)
	var type: int = -1
	var best_mag: int = -1
	for type_to_check: int in resources:
		var mag: int = resources[type_to_check]
		if type_to_check == 10 or type_to_check == 11:
			@warning_ignore("narrowing_conversion")
			mag = (mag * 0.7)
		if best_mag < mag:
			best_mag = mag
			type = type_to_check
	return type

func get_available_resources(coords: Vector2i) -> Dictionary:
	var toReturn: Dictionary = {}
	for type: int in get_child_count():
		var mag: int = get_tile_magnitude(coords, type)
		if mag > 0:
			toReturn[type] = mag
	return toReturn

func open_resource_map(type: int) -> void:
	get_layer(type).visible = true

func close_all_layers() -> void:
	for i: int in terminal_map.get_instance().amount_of_primary_goods:
		get_layer(i).visible = false

func get_tile_magnitude(coords: Vector2i, type: int) -> int:
	mutex.lock()
	var layer: TileMapLayer = get_layer(type)
	mutex.unlock()
	assert(layer != null)
	var toReturn: int = 0
	var atlas: Vector2i = layer.get_cell_atlas_coords(coords)
	if atlas != Vector2i(-1, -1):
		toReturn = atlas.y * TILES_PER_ROW + atlas.x
	return toReturn

func get_atlas_for_magnitude(num: int) -> Vector2i:
	@warning_ignore("integer_division")
	return Vector2i(num % TILES_PER_ROW, num / TILES_PER_ROW)

func get_good_name_uppercase(type: int) -> String:
	var cargo_name: String = CargoInfo.get_instance().get_cargo_name(type)
	cargo_name[0] = cargo_name[0].to_upper()
	return cargo_name

func get_available_primary_recipes(coords: Vector2i) -> Array[Array]:
	var toReturn: Array[Array] = []
	for type: int in get_child_count():
		if can_build_type(type, coords):
			var dict: Dictionary = {}
			dict[type] = 1
			toReturn.append([{}, dict])
	return toReturn

func place_resources(_map: TileMapLayer) -> void:
	map = _map
	var helper: RefCounted = load("res://Map/Cargo_Maps/cargo_values_helper.gd").new(map)
	var resource_array: Array = helper.create_resource_array()
	assert(resource_array != null or resource_array.is_empty(), "Resources generated improperly")
	var threads: Array = []
	
	for i: int in get_child_count():
		var thread: Thread = Thread.new()
		threads.append(thread)
		thread.start(autoplace_resource.bind(resource_array[i], i, MAX_RESOURCES[i]))
	for thread: Thread in threads:
		thread.wait_to_finish()
	finished_created_map_resources.emit()

func create_provinces_and_pop() -> void:
	create_territories()
	place_population()

func autoplace_resource(tiles: Dictionary, type: int, max_resouces: int) -> void:
	var array: Array = tiles.keys()
	array.shuffle()
	var count: int = 0
	for cell: Vector2i in array:
		
		var mag: int = randi() % 4 + tiles[cell]
		set_resource_locally(type, cell, get_atlas_for_magnitude(mag))
		call_deferred("set_resource_rpc", type, cell, get_atlas_for_magnitude(mag))
		count += mag
		if count > max_resouces and max_resouces != -1:
			return

func place_population() -> void:
	var helper: RefCounted = load("res://Map/Cargo_Maps/population_helper.gd").new()
	helper.create_population_map()

func create_territories() -> void:
	refresh_territories()

#Adds provinces and provinces, used for creating territories from multiple colors
func create_territory(start: Vector2i, provinces: TileMapLayer) -> Array:
	var atlas: Vector2i = provinces.get_cell_atlas_coords(start)
	var visited: Dictionary = {}
	visited[start] = 0
	var toReturn: Array = [start]
	var queue: Array = [start]
	while !queue.is_empty():
		var curr: Vector2i = queue.pop_front()
		for tile: Vector2i in provinces.get_surrounding_cells(curr):
			if !visited.has(tile) and provinces.get_cell_atlas_coords(tile) == atlas:
				visited[tile] = 0
				toReturn.push_back(tile)
				queue.push_back(tile)
			elif is_tile_water_and_real(tile):
				if visited[curr] < 5 and !visited.has(tile):
					visited[tile] = visited[curr] + 1
					queue.push_back(tile)
				elif visited.has(tile) and visited[tile] > visited[curr] + 1:
					visited[tile] = visited[curr] + 1
					queue.push_back(tile)
	return toReturn

#Uses image to re-create provinces tilemaplayer, then remakes image to match tilemaplater
func refresh_territories() -> void:
	var map_data_obj: map_data = map_data.get_instance()
	#Uses states for now, may change
	var file: String = "res://Map/Map_Info/states.png"
	var im_provinces: Image = load(file).get_image()
	#For Testing
	#var provinces: TileMapLayer = preload("res://Map/Map_Info/provinces.tscn").instantiate()
	#add_child(provinces)
	var colors_to_province_id: Dictionary = {}
	var province_id_to_color: Dictionary = {}
	var current_prov_id: int = 0
	create_colors_to_province_id(colors_to_province_id)
	for real_x: int in range(-609, 671):
		for real_y: int in range(-243, 282):
			@warning_ignore("integer_division")
			var x: int = (real_x + 609) * 3 / 2
			@warning_ignore("integer_division")
			var y: int = (real_y + 243) * 7 / 4
			var tile: Vector2i = Vector2i(real_x, real_y)
			var color: Color = im_provinces.get_pixel(x, y)
			if Utils.is_tile_water(tile):
				continue
			if !colors_to_province_id.has(color):
				colors_to_province_id[color] = current_prov_id
				province_id_to_color[current_prov_id] = color
				current_prov_id += 1
			#provinces.add_tile_to_province(tile, colors_to_province_id[color])
			map_data_obj.create_new_if_empty(colors_to_province_id[color])
			map_data_obj.add_tile_to_province(colors_to_province_id[color], tile)

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
			var prov_id: int = map_data_obj.get_province_id(tile)
			var color: Color = province_id_to_color[prov_id]
			new_image.set_pixel(x, y, color)
			if x != 0 and y != 0:
				new_image.set_pixel(x - 1, y - 1, color)
			if y != 0:
				new_image.set_pixel(x, y - 1, color)
			if x != 0:
				new_image.set_pixel(x - 1, y, color)
	
	new_image.save_png(file)

func use_image_to_create_unique_province_colors() -> void:
	var map_data_obj: map_data = map_data.get_instance()
	var new_image: Image = Image.create(1920, 919, false, Image.FORMAT_RGBA8)
	var colors: Dictionary[Color, bool] = {}
	var prov_id_to_color: Dictionary[int, Color] = {}
	for prov: Province in map_data_obj.get_provinces():
		var prov_id: int = prov.get_province_id()
		for tile: Vector2i in prov.get_tiles():
			@warning_ignore("integer_division")
			var x: int = (tile.x + 609) * 3 / 2
			@warning_ignore("integer_division")
			var y: int = (tile.y + 243) * 7 / 4
			var color: Color = get_color(prov_id)
			#Could potentially double map a color
			if colors.has(color) and !prov_id_to_color.has(prov_id):
				assert(false)
			prov_id_to_color[prov_id] = color
			colors[color] = true
			new_image.set_pixel(x, y, color)
			if x != 0 and y != 0:
				new_image.set_pixel(x - 1, y - 1, color)
			if y != 0:
				new_image.set_pixel(x, y - 1, color)
			if x != 0:
				new_image.set_pixel(x - 1, y, color)
	var file: String = "res://Map/Map_Info/provinces.png"
	new_image.save_png(file)

func get_color(prov_id: int) -> Color:
	var id: int = (prov_id + 31) * 53
	return Color((int(id * 1.1) % 255) / 255.0, ((int(id * 1.2) + 100) % 255) / 255.0, ((int(id * 1.3) + 200) % 255) / 255.0)

func get_closest_color(color: Color) -> Color:
	var red: float = 0.0
	var blue: float = 0.0
	var green: float = 0.0
	if color.r > 0.45 and color.r < 0.55:
		red = 0.5
	if color.g > 0.45 and color.g < 0.55:
		green = 0.5
	if color.b > 0.45 and color.b < 0.55:
		blue = 0.5
	if red == blue and blue == green and red == 0.0:
		return color
	return Color(red, green, blue)

func create_colors_to_province_id(colors_to_province_id: Dictionary) -> void:
	colors_to_province_id[Color(1, 0, 0, 1)] = 0
	colors_to_province_id[Color(0, 1, 0, 1)] = 1
	colors_to_province_id[Color(0, 0, 1, 1)] = 2
	colors_to_province_id[Color(0.5, 0.5, 0, 1)] = 3
	colors_to_province_id[Color(0.5, 0, 0.5, 1)] = 4
	colors_to_province_id[Color(0, 0.5, 0.5, 1)] = 5

func is_tile_water_and_real(coords: Vector2i) -> bool:
	var atlas: Vector2i = map.get_cell_atlas_coords(coords)
	return atlas == Vector2i(6, 0) or atlas == Vector2i(7, 0)

#func create_continents():
	#var file = FileAccess.open("res://Map/Map_Info/North_America.txt", FileAccess.WRITE)
	#for tile in $Layer1Sand.get_used_cells():
		#if !is_tile_water(map, tile):
			#save_to_file(file, str(tile) + '.')
	#file.close()
	#file = FileAccess.open("res://Map/Map_Info/South_America.txt", FileAccess.WRITE)
	#for tile in $Layer2Sulfur.get_used_cells():
		#if !is_tile_water(map, tile):
			#save_to_file(file, str(tile) + '.')
	#file.close()
	#file = FileAccess.open("res://Map/Map_Info/Europe.txt", FileAccess.WRITE)
	#for tile in $Layer3Lead.get_used_cells():
		#if !is_tile_water(map, tile):
			#save_to_file(file, str(tile) + '.')
	#file.close()
	#file = FileAccess.open("res://Map/Map_Info/Africa.txt", FileAccess.WRITE)
	#for tile in $Layer4Iron.get_used_cells():
		#if !is_tile_water(map, tile):
			#save_to_file(file, str(tile) + '.')
	#file.close()
	#file = FileAccess.open("res://Map/Map_Info/Asia.txt", FileAccess.WRITE)
	#for tile in $Layer5Coal.get_used_cells():
		#if !is_tile_water(map, tile):
			#save_to_file(file, str(tile) + '.')
	#file.close()
	#file = FileAccess.open("res://Map/Map_Info/Australia.txt", FileAccess.WRITE)
	#for tile in $Layer6Copper.get_used_cells():
		#if !is_tile_water(map, tile):
			#save_to_file(file, str(tile) + '.')
	#file.close()
	#file = FileAccess.open("res://Map/Map_Info/Australia.txt", FileAccess.READ)
	#try_to_read(file)
#
#func save_to_file(file, content: String):
	#file.store_string(content)
	#
#
#func try_to_read(file: FileAccess):
	#var packedString = file.get_csv_line('.')
	#print(packedString[0])
	#print(packedString[999])
	#
