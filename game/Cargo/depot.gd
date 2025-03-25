extends terminal
var trains = []

var map: TileMapLayer
const train_scene = preload("res://Cargo/Cargo_Objects/train.tscn")


func _init(new_location: Vector2i, _owner: int, new_map):
	super._init(new_location, _owner)
	map = new_map

func get_depot_name() -> String:
	return "Depot"

func get_trains():
	return trains

func get_trains_simplified() -> Array:
	var toReturn = []
	for train in trains:
		toReturn.append(train.name)
	return toReturn

func get_train(index: int):
	return trains[index]

func add_train(train):
	trains.append(train)

func add_new_train():
	var train = train_scene.instantiate()
	map.add_child(train)
	train.create(location)
	trains.append(train)
	train.visible = false

func leave_depot(index: int):
	var dir = map.get_depot_direction(location)
	var train = trains.pop_at(index)
	train.go_out_of_depot.rpc(dir)

func remove_train(index: int):
	trains.remove_at(index)
