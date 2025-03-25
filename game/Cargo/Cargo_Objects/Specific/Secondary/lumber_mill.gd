extends factory

func _init(new_location: Vector2i, _player_owner: int):
	var new_inputs = {}
	new_inputs[terminal_map.get_cargo_type("wood")] = 2
	var new_outputs = {}
	new_outputs[terminal_map.get_cargo_type("lumber")] = 1
	super._init(new_location, _player_owner, new_inputs, new_outputs)
