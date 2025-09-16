class_name train_manager extends Node

const train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/Trains/train.tscn")
const ai_train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/Trains/ai_train.tscn")

var networks: Dictionary[int, rail_network] #Each network id points to network
var trains: Dictionary[int, train] = {}
var owned_trains: Dictionary[int, Dictionary] #Links player/ai ids to a set of train_ids they own

static var singleton_instance: train_manager

func _init() -> void:
	assert(singleton_instance == null, "Cannot create multiple instances of singleton!")
	singleton_instance = self

static func get_instance() -> train_manager:
	assert(singleton_instance != null, "Train_Manager has not be created, and has been accessed")
	return singleton_instance

static func has_instance() -> bool:
	return singleton_instance != null

func get_network(network_id: int) -> rail_network:
	return networks[network_id]

func create_train(p_location: Vector2i, p_owner: int) -> void:
	var id: int = get_unique_id()
	var new_train: train = train_scene.instantiate()
	new_train.create(p_location, p_owner, id)
	new_train.name = "Train" + str(trains.size())
	Utils.world_map.add_child(new_train)
	add_train(new_train)

func create_ai_train(p_location: Vector2i, p_owner: int) -> ai_train:
	var id: int = get_unique_id()
	var new_train: ai_train = ai_train_scene.instantiate()
	new_train.create(p_location, p_owner, id)
	new_train.name = "AI_Train" + str(trains.size())
	Utils.world_map.add_child(new_train)
	add_ai_train(new_train)
	return new_train

func get_unique_id() -> int:
	var toReturn: int = randi() % 100000000
	while trains.has(toReturn):
		toReturn = randi() % 100000000
	return toReturn

func add_train(p_train: train) -> void:
	trains[p_train.id] = p_train
	var p_owner: int = p_train.player_owner
	if !owned_trains.has(p_owner):
		owned_trains[p_owner] = {}
	owned_trains[p_owner][p_train.id] = true

func add_ai_train(p_train: ai_train) -> void:
	add_train(p_train)
	#Assigning ai train to random 'Network' unless it matches with another train
	var id: int = get_network_id(p_train)
	add_ai_train_to_network(p_train, id)
	if !networks[p_train.network_id].is_built():
		create_network_for_trains(id)
	networks[p_train.network_id].split_up_network_among_trains()

func get_network_id(p_train: ai_train) -> int:
	for network_id: int in networks:
		var network: rail_network = networks[network_id]
		if network.has_coords(p_train.location):
			return network_id
	return get_unique_id()

func add_ai_train_to_network(train_obj: ai_train, network_id: int) -> void:
	train_obj.network_id = network_id
	if !networks.has(network_id):
		networks[network_id] = rail_network.new(network_id)
	networks[network_id].add_train_to_network(train_obj.id)

func get_member_from_network(network_id: int) -> ai_train:
	return get_ai_train(networks[network_id].get_member_from_network())

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
	return networks[network_id].get_number_of_trains()

#Automatically recreates rail_network anytime a train is added which is uneccessary but will change later
func create_network_for_trains(network_id: int) -> void:
	if !networks.has(network_id):
		networks[network_id] = rail_network.new(network_id)
	var network: rail_network = networks[network_id]
	#Will reset and re-create network
	network.create_network(get_member_from_network(network_id).location)

func process(delta: float) -> void:
	for train_obj: train in trains.values():
		train_obj.process(delta)
