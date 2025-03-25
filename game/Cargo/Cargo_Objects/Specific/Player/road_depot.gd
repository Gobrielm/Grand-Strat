class_name road_depot extends station

var supplied_tiles = {}

func _init(coords: Vector2i, _player_owner: int, new_supply_map: Dictionary):
	super._init(coords, _player_owner)
	supplied_tiles = new_supply_map

func get_supply(coords: Vector2i) -> int:
	if supplied_tiles.has(coords):
		return supplied_tiles[coords]
	return 0

func distribute_cargo():
	for type in storage:
		distribute_type(type)

func distribute_type(type: int):
	for tile: Vector2i in supplied_tiles:
		var hold_obj = terminal_map.get_terminal(tile)
		if hold_obj != null and hold_obj is hold:
			if hold_obj.does_accept(type) and hold_obj.get_player_owner() == player_owner:
				distribute_type_to_hold(type, hold_obj)

func distribute_type_to_hold(type: int, hold_obj: hold):
	var coords = hold_obj.get_location()
	var amount = hold_obj.get_amount_to_add(type, supplied_tiles[coords])
	amount = transfer_cargo(type, amount)
	hold_obj.add_cargo(type, amount)
