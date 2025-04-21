class_name train_manager extends Node

const train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/train.tscn")
const ai_train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/ai_train.tscn")

var network_members: Dictionary[int, Array] #Array[int], network id -> array of ai_train ids

var networks: Dictionary[int, Dictionary] #Dictionary[Vector2i, rail_node], each network id points to network
var trains: Dictionary[int, train] = {}


static var singleton_instance: train_manager

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> train_manager:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

func create_train(p_location: Vector2i, p_owner: int) -> train:
	var id: int = get_unique_id()
	var new_train: train = train_scene.instantiate()
	new_train.create(p_location, p_owner, id)
	add_train(new_train)
	return new_train

func create_ai_train(p_location: Vector2i, p_owner: int) -> ai_train:
	var id: int = get_unique_id()
	var new_train: ai_train = ai_train_scene.instantiate()
	new_train.create(p_location, p_owner, id)
	add_ai_train(new_train)
	return new_train

func get_unique_id() -> int:
	var toReturn: int = randi() % 100000000
	while trains.has(toReturn):
		toReturn = randi() % 100000000
	return toReturn

func add_train(p_train: train) -> void:
	trains[p_train.id] = p_train

func add_ai_train(p_train: ai_train) -> void:
	add_train(p_train)
	#Assigning ai train to random 'Network' unless it matches with another train
	add_ai_train_to_network(p_train, get_network(p_train))

func get_network(p_train: ai_train) -> int:
	for network_id: int in network_members:
		#Getting one train in network
		var train_obj: ai_train = get_member_from_network(network_id)
		if network_has_train(train_obj, p_train):
			return train_obj.network_id
	return get_unique_id()

#Assumes that train2 is on a valid rail vertex to work
func network_has_train(train1: ai_train, train2: ai_train) -> bool:
	if networks[train1.network_id].has(train2.location):
		return true
	return false

func add_ai_train_to_network(train_obj: ai_train, network_id: int) -> void:
	train_obj.network_id = network_id
	network_members[network_id].append(train_obj)

func get_member_from_network(network_id: int) -> ai_train:
	return get_ai_train(network_members[network_id].front())

func get_train(id: int) -> train:
	if trains.has(id):
		return trains[id]
	return null

func get_ai_train(id: int) -> ai_train:
	if trains.has(id) and trains[id] is ai_train:
		return trains[id]
	return null

#Will ignore regular trains
func get_trains_on_network(network_id: int) -> int:
	return network_members[network_id].size()

#Automatically recreates rail_network anytime a train is added which is uneccessary but will change later
func create_network_for_trains(network_id: int) -> void:
	#TODO: Make sure network doesn't reset for a new train
	create_network(get_member_from_network(network_id).location, network_id)
	split_up_network_among_trains(networks[network_id], network_members[network_id])
	

func create_network(start: Vector2i, network_id: int) -> void:
	var map: TileMapLayer = Utils.world_map
	var queue: Array = [start]
	var visited: Dictionary = {} # Vector2i -> Array[Bool for each direction]
	var dist: Dictionary[Vector2i, Array] = {} #Array[Vector2i(Source), int(dist)]
	visited[start] = Utils.rail_placer.get_track_connections(start)
	var curr: Vector2i
	var rail_network: Dictionary[Vector2i, rail_node] = {}
	while !queue.is_empty():
		curr = queue.pop_front()
		var is_node: bool = terminal_map.is_station(curr) or map_data.get_instance().is_depot(curr) or Utils.rail_placer.get_track_connection_count(curr) >= 3
		if is_node:
			create_node(curr, rail_network)
			if dist.has(curr):
				connect_nodes(curr, dist[curr][0], dist[curr][1], rail_network)
			dist[curr] = [curr, 0]
		
		var cells_to_check: Array = get_cells_in_front(curr, visited[curr], map)
		for direction: int in cells_to_check.size():
			var tile: Vector2i = cells_to_check[direction]
			if tile != null and map.do_tiles_connect(curr, tile) and !check_visited(visited, tile, direction):
				intialize_visited(visited, tile, direction)
				queue.push_back(tile)
				dist[tile] = dist[curr]
				dist[tile][1] += 1
	networks[network_id] = rail_network

func get_cells_in_front(coords: Vector2i, directions: Array, map: TileMapLayer) -> Array:
	var index: int = 2
	var toReturn: Array = [null, null, null, null, null, null]
	for cell: Vector2i in map.get_surrounding_cells(coords):
		if directions[index] or directions[(index + 1) % 6] or directions[(index + 5) % 6]:
			toReturn[index] = cell
		index = (index + 1) % 6
	return toReturn

func intialize_visited(visited: Dictionary, coords: Vector2i, direction: int) -> void:
	if !visited.has(coords):
		visited[coords] = [false, false, false, false, false, false]
	visited[coords][direction] = true

func check_visited(visited: Dictionary, coords: Vector2i, direction: int) -> bool:
	if visited.has(coords):
		return visited[coords][direction]
	return false

func create_node(coords: Vector2i, rail_network: Dictionary[Vector2i, rail_node]) -> void:
	if !rail_network.has(coords):
		rail_network[coords] = rail_node.new(coords)

func connect_nodes(coords1: Vector2i, coords2: Vector2i, dist: int, rail_network: Dictionary[Vector2i, rail_node]) -> void:
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

func split_up_network_among_trains(rail_network: Dictionary[Vector2i, rail_node], train_members: Array[int]) -> void:
	var number_of_trains: int = train_members.size()
	
