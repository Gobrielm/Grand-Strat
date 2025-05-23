extends TileMapLayer

var mutex: Mutex = Mutex.new()

func _ready() -> void:
	name = "cargo_map"
	Utils.assign_cargo_map(self)

@rpc("authority", "call_local", "unreliable")
func set_tile(coords: Vector2i, atlas: Vector2i) -> void:
	mutex.lock()
	set_cell(coords, 0, atlas)
	mutex.unlock()
	Utils.world_map.make_cell_invisible(coords)
