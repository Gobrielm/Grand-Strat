class_name train_manager extends Node

const train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/train.tscn")
const ai_train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/ai_train.tscn")

var ai_trains: Dictionary[int, ai_train] = {}
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
	ai_trains[p_train.id] = p_train

func get_train(id: int) -> train:
	if trains.has(id):
		return trains[id]
	return null

func get_ai_train(id: int) -> ai_train:
	if ai_trains.has(id):
		return ai_trains[id]
	return null
