class_name ai_train extends train

var rail_network: Dictionary[Vector2i, rail_node] = {}

func create_rail_graph_network() -> void:
	create_network(location)
	var number_trains: int = get_trains_on_network()

func create_network(start: Vector2i) -> void:
	var queue: Array = [start]
	var visited: Dictionary = {} # Vector2i -> Array[Bool for each direction]
	var dist: Dictionary[Vector2i, Array] = {} #Array[Vector2i(Source), int(dist)]
	visited[start] = get_train_dir_in_array()
	var curr: Vector2i
	while !queue.is_empty():
		curr = queue.pop_front()
		var is_node: bool = terminal_map.is_station(curr) or map_data.get_instance().is_depot(curr) or Utils.rail_placer.get_track_connection_count(curr) >= 3
		if is_node:
			create_node(curr)
			if dist.has(curr):
				connect_nodes(curr, dist[curr][0], dist[curr][1])
			dist[curr] = [curr, 0]
		
		var cells_to_check: Array = get_cells_in_front(curr, visited[curr])
		for direction: int in cells_to_check.size():
			var tile: Vector2i = cells_to_check[direction]
			if tile != null and map.do_tiles_connect(curr, tile) and !check_visited(visited, tile, direction):
				intialize_visited(visited, tile, direction)
				queue.push_back(tile)
				dist[tile] = dist[curr]
				dist[tile][1] += 1

func create_node(coords: Vector2i) -> void:
	if !rail_network.has(coords):
		rail_network[coords] = rail_node.new(coords)

func connect_nodes(coords1: Vector2i, coords2: Vector2i, dist: int) -> void:
	if rail_network.has(coords1) and rail_network.has(coords2):
		var weight: float = get_weight_of_route(coords1, coords2, dist)
		rail_network[coords1].connect_nodes(coords2, weight)
		rail_network[coords2].connect_nodes(coords1, weight)

func get_weight_of_route(coords1: Vector2i, coords2: Vector2i, dist: int) -> float:
	if terminal_map.is_station(coords1) and terminal_map.is_station(coords2):
		var stat1: station = terminal_map.get_station(coords1)
		var stat2: station = terminal_map.get_station(coords2)
		return stat1.get_orders_magnitude() + stat2.get_orders_magnitude() + 5.0 / dist
	elif terminal_map.is_station(coords1) or terminal_map.is_station(coords2):
		var stat: station = terminal_map.get_station(coords1)
		if stat == null:
			stat = terminal_map.get_station(coords2)
		return stat.get_orders_magnitude() + 3.0 / dist
	else:
		return 5 + 2.0 / dist

func get_trains_on_network() -> int:
	var count: int = 1
	for train_obj: train in Utils.world_map.get_trains():
		if compare_networks(train_obj):
			count += 1
	return count

func compare_networks(other_train: train) -> bool:
	if other_train is ai_train:
		if (other_train as ai_train).rail_network.has(location):
			return true
	return false
