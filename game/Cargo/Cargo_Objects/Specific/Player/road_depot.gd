class_name road_depot extends station

#TODO: Make road depot have orders have be way more similar to station

var supplied_tiles: Dictionary[Vector2i, int] = {}

const MAX_DISTANCE: int = 5
#If distance is 5 then 5 * 1 = 5 cargo delievered
const CARGO_DELIEVERED_PER_UNIT_DISTANCE: int = 1

func _init(coords: Vector2i, _player_owner: int) -> void:
	super._init(coords, _player_owner)
	supplied_tiles = create_supplied_tiles(coords)

func create_supplied_tiles(center: Vector2i) -> Dictionary[Vector2i, int]:
	var map: TileMapLayer = Utils.world_map
	var toReturn: Dictionary[Vector2i, int] = {}
	toReturn[center] = MAX_DISTANCE
	var visited: Dictionary[Vector2i, bool] = {}
	visited[center] = true
	var queue: Array = [center]
	while !queue.is_empty():
		var curr: Vector2i = queue.pop_front()
		var tiles: Array = map.get_surrounding_cells(curr)
		for tile: Vector2i in tiles:
			if !visited.has(tile) and toReturn[curr] > 0:
				visited[tile] = true
				queue.push_back(tile)
				toReturn[tile] = toReturn[curr] - 1
	return toReturn

func get_supply(coords: Vector2i) -> int:
	if supplied_tiles.has(coords):
		return supplied_tiles[coords]
	return 0

func distribute_cargo() -> void:
	for type: int in storage:
		distribute_type(type)

func distribute_type(type: int) -> void:
	for tile: Vector2i in supplied_tiles:
		var broker_obj: broker = terminal_map.get_broker(tile)
		if broker_obj != null and broker_obj.does_accept(type):
			#Only sends stuff inside country
			if tile_ownership.get_instance().is_owned(player_owner, broker_obj.get_location()):
				distribute_type_to_broker(type, broker_obj)

func distribute_type_to_broker(type: int, broker_obj: broker) -> void:
	var coords: Vector2i = broker_obj.get_location()
	var amount: int = broker_obj.get_amount_to_add(type, supplied_tiles[coords])
	amount = transfer_cargo(type, amount)
	broker_obj.add_cargo(type, amount)
