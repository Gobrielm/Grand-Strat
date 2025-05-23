extends Node2D

var magnitude_layers: Array = []

var mutex: Mutex = Mutex.new()

func _ready() -> void:
	name = "cargo_values"
	create_magnitude_layers()

func create_magnitude_layers() -> void:
	for child: Node in get_children():
		if child is TileMapLayer:
			magnitude_layers.append(child)

@rpc("authority", "call_local", "reliable")
func set_cell_rpc(type: int, coords: Vector2i, atlas: Vector2i) -> void:
	get_layer(type).set_cell(coords, 1, atlas)

func get_layer(type: int) -> TileMapLayer:
	mutex.lock()
	var layer: TileMapLayer = magnitude_layers[type]
	mutex.unlock()
	assert(layer != null and layer.name == ("Layer" + str(type) + get_good_name_uppercase(type)))
	return layer

func get_good_name_uppercase(type: int) -> String:
	var cargo_name: String = terminal_map.get_cargo_name(type)
	cargo_name[0] = cargo_name[0].to_upper()
	return cargo_name

func close_all_layers() -> void:
	for i: int in terminal_map.amount_of_primary_goods:
		get_layer(i).visible = false

func open_resource_map(type: int) -> void:
	get_layer(type).visible = true
