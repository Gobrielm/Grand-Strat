extends Node2D


var tile_metadata: Dictionary = {
	#TileID
		#0: City, 1: Depot, 2: Station
	#Name
		#String
	#Size
		#Population: int, Yearly production: int
	#Map
		#Only for cities, ptr -> depot
}

var non_buildable_tiles: Dictionary = {
	
}

func _process(_delta):
	pass


# Called when the node enters the scene tree for the first time.
func _ready():
	tile_metadata[Vector2i(0, -3)] = [0, "Madrid", 330, generate_city()]
	tile_metadata[Vector2i(-11, 0)] = [0, "Lisbon", 57, generate_city()]
	tile_metadata[Vector2i(19, -29)] = [0, "Paris", 210, generate_city()]
	
	#Unbuildable tiles
	non_buildable_tiles[Vector2i(6, 0)] = 0
	non_buildable_tiles[Vector2i(7, 0)] = 0
	non_buildable_tiles[Vector2i(0, 1)] = 0


@rpc("authority", "call_local", "reliable")
func update_tile_metadata(tile_coordinates : Vector2i, new_tile_metadata):
	tile_metadata[tile_coordinates] = new_tile_metadata

func get_tile_metadata(tile_coordinates : Vector2i):
	if tile_metadata.has(tile_coordinates):
		return tile_metadata[tile_coordinates]
	return null

func is_location_depot(tile_coordinates : Vector2i) -> bool:
	return tile_metadata.has(tile_coordinates) and tile_metadata[tile_coordinates][0] == 1

@rpc("authority", "call_local", "reliable")
func delete_tile_metadata(tile_coordinates : Vector2i):
	tile_metadata.erase(tile_coordinates)

func can_place_here(atlas_coords: Vector2i):
	return !non_buildable_tiles.has(atlas_coords)

func get_tile_price(_atlas_coords: Vector2i, _source: int):
	#FIXME
	return 0

func generate_city():
	var city = create_city_map()
	build_roads(city)
	return city

func create_city_map():
	var city = []
	for i in 100:
		city.append([])
		for j in 100:
			city[i].append(0)
	return city

func build_roads(city):
	var rand = RandomNumberGenerator.new()
	build_outer_roads(city)
	build_inner_inner_roads(rand, city, build_inner_roads(rand, city))

func build_outer_roads(city):
	for x in 40:
		for y in 40:
			if x == 0 or x == 39 or y == 0 or y == 39:
				city[x + 30][y + 30] = 1

func build_inner_roads(rand: RandomNumberGenerator, city):
	var road_splits = []
	for i in 6:
		var x = rand.randi_range(30, 70)
		for y in range(30, 70):
			if rand.randi_range(0, 30) == 0:
				city[x][y] = 2
				road_splits.append(Vector2i(x, y))
				if rand.randi_range(0, 2) == 0:
					break
				
			city[x][y] = 1
	return road_splits

func build_inner_inner_roads(rand: RandomNumberGenerator, city, split_locations): 
	for coords in split_locations:
		var rand_num = rand.randi_range(0, 3)
		if rand_num == 0:
			for x in range(coords.x, 70):
				if city[x][coords.y] == 1 and rand.randi_range(0, 3) == 0:
					break
				city[x][coords.y] = 1
		elif rand_num == 1:
			for y in range(coords.y, 70):
				if city[coords.x][y] == 1 and rand.randi_range(0, 3) == 0:
					break
				city[coords.x][y] = 1
		else:
			for x in range(coords.x, 29, -1):
				if city[x][coords.y] == 1 and rand.randi_range(0, 3) == 0:
					break
				city[x][coords.y] = 1
		
	
	
	
