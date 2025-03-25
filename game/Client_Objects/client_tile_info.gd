extends Node

var map: TileMapLayer

var depots = {}

var holds = {}

func _init(new_map: TileMapLayer):
	map = new_map

func add_depot(coords: Vector2i, depot_name: String, player_id: int):
	depots[coords] = [depot_name, player_id]

func get_depot_name(coords: Vector2i) -> String:
	if is_depot(coords):
		return depots[coords][0]
	return ""

func is_depot(coords: Vector2i) -> bool:
	return depots.has(coords)

func is_owned_depot(coords: Vector2i, id: int) -> bool:
	return depots.has(coords) and depots[coords][1] == id

func add_hold(coords: Vector2i, hold_name: String, player_id: int):
	holds[coords] = [hold_name, player_id]

func get_hold_name(coords: Vector2i) -> String:
	if holds.has(coords):
		return holds[coords][0]
	return ""

func is_hold(coords: Vector2i) -> bool:
	return holds.has(coords)

func is_owned_hold(coords: Vector2i, id: int) -> bool:
	return holds.has(coords) and holds[coords][1] == id
