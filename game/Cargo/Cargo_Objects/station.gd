class_name Station extends StationWOMethods

func _init(p_location: Vector2i, p_owner: int) -> void:
	super.initialize(p_location, p_owner)

func supply_armies() -> void:
	var units_to_supply: Dictionary[Vector2i, int] = get_units_to_supply()
	if units_to_supply.size() > 0:
		print("supplying unit")
	for tile: Vector2i in units_to_supply:
		var storage: Dictionary = get_current_hold()
		for type: int in storage:
			if storage[type] == 0:
				continue
			supply_army(tile, type, units_to_supply[tile])

func get_units_to_supply() -> Dictionary[Vector2i, int]:
	var map: TileMapLayer = Utils.world_map
	var unit_map: TileMapLayer = Utils.unit_map
	var visited: Dictionary = {}
	visited[get_location()] = MAX_SUPPLY_DISTANCE
	var queue: Array = [get_location()]
	var units_to_supply: Dictionary[Vector2i, int] = {}
	while !queue.is_empty():
		var curr: Vector2i = queue.pop_front()
		for tile: Vector2i in map.get_surrounding_cells(curr):
			if !visited.has(tile) or (visited.has(tile) and visited[tile] < visited[curr] - SUPPLY_DROPOFF):
				#Can't supply through enemy units
				if unit_map.tile_has_enemy_army(tile, player_owner):
					continue
				visited[tile] = visited[curr] - SUPPLY_DROPOFF
				if visited[tile] > 0:
					queue.push_back(tile)
					if unit_map.tile_has_friendly_army(tile, player_owner):
						units_to_supply[tile] = visited[tile]
	return units_to_supply

func supply_army(tile: Vector2i, type: int, max_supply: int) -> void:
	var army_obj: army = Utils.unit_map.get_army(tile, player_owner)
	if army_obj == null:
		return
	for unit: base_unit in army_obj.get_units():
		supply_unit(unit, type, max_supply)
	
func supply_unit(unit: base_unit, type: int, max_supply: int) -> void:
	var org: organization = unit.org
	var desired: int = min(org.get_desired_cargo(type), max_supply)
	var amount: int = min(get_cargo_amount(type), desired)
	remove_cargo(type, amount)
	org.add_cargo(type, amount)
	#TODO: Do some storing of, amount spent/goods used, on war
