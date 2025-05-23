extends tile_ownership

func _ready() -> void:
	singleton_instance = self
	prepare_refresh_tile_ownership.rpc_id(1)

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

@rpc("any_peer", "call_local", "reliable")
func select_nation(cells_to_change: Array) -> void:
	var atlas: Vector2i = get_cell_atlas_coords(cells_to_change[0])
	for cell: Vector2i in cells_to_change:
		set_cell(cell, 1, atlas)

@rpc("authority", "call_local", "reliable")
func play_noise() -> void:
	$click_noise.play()

@rpc("any_peer", "call_local", "reliable")
func unselect_nation(cells_to_change: Array) -> void:
	var atlas: Vector2i = get_cell_atlas_coords(cells_to_change[0])
	for cell: Vector2i in cells_to_change:
		set_cell(cell, 0, atlas)

@rpc("any_peer", "call_local", "unreliable")
func add_player_to_country(_player_id: int, _coords: Vector2i) -> void:
	pass
