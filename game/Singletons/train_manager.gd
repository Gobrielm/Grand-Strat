class_name train_manager extends Node

const train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/train.tscn")
const ai_train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/ai_train.tscn")

var networks: Dictionary[int, rail_network] #Each network id points to network
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
	for network_id: int in networks:
		#Getting one train in network
		var train_obj: ai_train = get_member_from_network(network_id)
		if network_has_train(train_obj, p_train):
			return train_obj.network_id
	return get_unique_id()

#Assumes that train2 is on a valid rail vertex to work
func network_has_train(train1: ai_train, train2: ai_train) -> bool:
	if networks[train1.network_id].network_has_node(train2.location):
		return true
	return false

func add_ai_train_to_network(train_obj: ai_train, network_id: int) -> void:
	train_obj.network_id = network_id
	if !networks.has(network_id):
		networks[network_id] = rail_network.new()
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
	var network: rail_network = networks[network_id]
	#Will reset and re-create network
	network.create_network(get_member_from_network(network_id).location)
	network.split_up_network_among_trains()
