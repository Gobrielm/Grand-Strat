class_name warehouse extends supply_hub

func _init(coords: Vector2i, _player_owner: int) -> void:
	super._init(coords, _player_owner)
	change_max_storage(DEFAULT_MAX_STORAGE * 10)
