class_name ai_train extends train

var rail_network: Dictionary[Vector2i, Dictionary] = {} #Vector2i -> Dictionary[Vector2i, int] #Set of connections

func create_rail_graph_network() -> void:
	create_nodes(location)



func create_nodes(start: Vector2i) -> void:
	var queue: Array = [start]
	var tile_to_prev: Dictionary = {} # Vector2i -> Array[Tile for each direction]
	var order: Dictionary = {} # Vector2i -> Array[indices in order for tile_to_prev, first one is the fastest]
	var visited: Dictionary = {} # Vector2i -> Array[Bool for each direction]
	visited[start] = get_train_dir_in_array()
	var curr: Vector2i
	while !queue.is_empty():
		curr = queue.pop_front()
		if terminal_map.is_station(curr):
			create_node(curr)
		elif map_data.get_instance().is_depot(curr):
			create_node(curr)
		elif Utils.rail_placer.get_track_connection_count(curr) >= 3:
			create_node(curr)
		
		var cells_to_check: Array = get_cells_in_front(curr, visited[curr])
		for direction: int in cells_to_check.size():
			var tile: Vector2i = cells_to_check[direction]
			if tile != null and map.do_tiles_connect(curr, tile) and !check_visited(visited, tile, direction):
				intialize_visited(visited, tile, direction)
				queue.push_back(tile)
				intialize_tile_to_prev(tile_to_prev, tile, swap_direction(direction), curr)
				intialize_order(order, tile, swap_direction(direction))

func create_node(coords: Vector2i) -> void:
	if !rail_network.has(coords):
		rail_network[coords] = {}

func connect_node(coords1: Vector2i, coords2: Vector2i) -> void:
	rail_network[coords1][coords2] = 1

func disconnect_node(coords1: Vector2i, coords2: Vector2i) -> void:
	rail_network[coords1].erase(coords2)
