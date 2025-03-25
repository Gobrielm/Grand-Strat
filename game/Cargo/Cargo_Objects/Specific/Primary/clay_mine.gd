extends base_factory

func _init(new_location: Vector2i, _player_owner: int):
	var dict = {}
	dict[terminal_map.get_cargo_type("clay")] = 1
	super._init(new_location, _player_owner, dict)
