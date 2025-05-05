class_name supply_hub extends station

#TODO: Make road depot have orders have be way more similar to station
#Can buy, but only send cargo to units

var supplied_tiles: Dictionary[Vector2i, int] = {} #Vector2i -> Supply
var unit_map: TileMapLayer

const MAX_SUPPLY: int = 5
const SUPPLY_DROPOFF: int = 1

func _init(coords: Vector2i, _player_owner: int) -> void:
	super._init(coords, _player_owner)
	supplied_tiles = create_supplied_tiles(coords)
	unit_map = Utils.unit_map

func create_supplied_tiles(center: Vector2i) -> Dictionary[Vector2i, int]:
	var map: TileMapLayer = Utils.world_map
	var toReturn: Dictionary[Vector2i, int] = {}
	toReturn[center] = MAX_SUPPLY
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
				toReturn[tile] = toReturn[curr] - SUPPLY_DROPOFF
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
		var unit: base_unit = unit_map.get_unit(tile)
		if unit != null:
			#Max amount can deliever
			var supply: int = supplied_tiles[tile]
			var org: organization = unit.get_organization_object()
			distribute_type_to_org(type, org, supply)

func distribute_type_to_org(type: int, org: organization, supply: int) -> void:
	var amount: int = min(org.get_desired_cargo(type), supply)
	amount = transfer_cargo(type, amount)
	org.add_cargo(type, amount)

func place_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	if buy:
		super.place_order(type, amount, buy, max_price)

func edit_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	if buy:
		super.edit_order(type, amount, buy, max_price)
