extends tile_ownership

func _ready() -> void:
	singleton_instance = self

func create_countries() -> void:
	pass

@rpc("authority", "call_remote", "reliable")
func refresh_tile_ownership(resource: Dictionary) -> void:
	clear()
	for tile: Vector2i in resource:
		set_cell(tile, 0, resource[tile])

@rpc("any_peer", "call_local", "reliable")
func prepare_refresh_tile_ownership() -> void:
	pass

@rpc("any_peer", "call_local", "unreliable")
func add_player_to_country(_coords: Vector2i) -> void:
	pass
