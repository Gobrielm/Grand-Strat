extends terminal
var trains: Array = []

const train_scene: PackedScene = preload("res://Cargo/Cargo_Objects/train.tscn")

func _init(new_location: Vector2i, _owner: int) -> void:
	super._init(new_location, _owner)

func get_depot_name() -> String:
	return "Depot"

func get_trains() -> Array:
	return trains

func get_trains_simplified() -> Array:
	var toReturn: Array = []
	for train_obj: Sprite2D in trains:
		toReturn.append(train_obj.name)
	return toReturn

func get_train(index: int) -> Sprite2D:
	return trains[index]

func add_train(train_obj: Sprite2D) -> void:
	trains.append(train_obj)

func add_new_train() -> void:
	var train_obj: Sprite2D = train_scene.instantiate()
	Utils.world_map.add_child(train_obj)
	train_obj.create(location)
	trains.append(train_obj)
	train_obj.visible = false

func leave_depot(index: int) -> void:
	var dir: int = Utils.world_map.get_depot_direction(location)
	var train_obj: Sprite2D = trains.pop_at(index)
	train_obj.go_out_of_depot.rpc(dir)

func remove_train(index: int) -> void:
	trains.remove_at(index)
