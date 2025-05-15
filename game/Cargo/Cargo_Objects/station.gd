class_name station extends broker

var MAX_SUPPLY_DISTANCE: int = 5
var MAX_SUPPLY_GIVEN: int = 5
const SUPPLY_DROPOFF: int = 1

func _init(new_location: Vector2i, _player_owner: int) -> void:
	super._init(new_location, _player_owner)

#Gets the local market's price
func get_local_price(type: int) -> float:
	var amount_total: int = 0
	var market_price: float = 0.0 #Starts as domestic output becomes price
	for tile: Vector2i in connected_terminals:
		var brok: broker = terminal_map.get_broker(tile)
		if brok == null:
			continue
		var order: trade_order = brok.get_order(type)
		if order == null:
			continue
		amount_total += order.amount
		market_price += order.amount * brok.get_local_price(type)
			
	return market_price / amount_total

func place_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	add_accept(type)
	super.place_order(type, amount, buy, max_price)

func edit_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	add_accept(type)
	super.edit_order(type, amount, buy, max_price)

func get_orders_magnitude() -> int:
	var tot: int = 0
	for order: trade_order in trade_orders.values():
		tot += order.amount
	return tot

func remove_order(type: int) -> void:
	if trade_orders.has(type):
		remove_accept(type)
		trade_orders.erase(type)

func distribute_cargo() -> void:
	#Prioritize re-supplying friendly units
	supply_units()
	for order: trade_order in trade_orders.values():
		if order.is_sell_order():
			distribute_from_order(order)

func supply_units() -> void:
	var units_to_supply: Dictionary[Vector2i, int] = get_units_to_supply()
	if units_to_supply.size() > 0:
		print("supplying unit")
	for tile: Vector2i in units_to_supply:
		for type: int in storage:
			if storage[type] == 0:
				continue
			supply_unit(tile, type, units_to_supply[tile])

func get_units_to_supply() -> Dictionary[Vector2i, int]:
	var map: TileMapLayer = Utils.world_map
	var unit_map: TileMapLayer = Utils.unit_map
	var visited: Dictionary = {}
	visited[location] = MAX_SUPPLY_DISTANCE
	var queue: Array = [location]
	var units_to_supply: Dictionary[Vector2i, int] = {}
	while !queue.is_empty():
		var curr: Vector2i = queue.pop_front()
		for tile: Vector2i in map.get_surrounding_cells(curr):
			if !visited.has(tile) or (visited.has(tile) and visited[tile] < visited[curr] - SUPPLY_DROPOFF):
				#Can't supply through enemy units
				if unit_map.tile_has_enemy_unit(tile, player_owner):
					continue
				visited[tile] = visited[curr] - SUPPLY_DROPOFF
				if visited[tile] > 0:
					queue.push_back(tile)
					if unit_map.tile_has_friendly_unit(tile, player_owner):
						units_to_supply[tile] = visited[tile]
	return units_to_supply

func supply_unit(tile: Vector2i, type: int, max_supply: int) -> void:
	var unit: base_unit = Utils.unit_map.get_unit(tile, player_owner)
	if unit == null:
		return
	var org: organization = unit.org
	var desired: int = min(org.get_desired_cargo(type), max_supply)
	var amount: int = transfer_cargo(type, desired)
	org.add_cargo(type, amount)
	#TODO: Do some storing of, amount spent/goods used, on war

func add_connected_terminal(new_terminal: terminal) -> void:
	super.add_connected_terminal(new_terminal)
	update_accepts_from_trains()

func remove_connected_terminal(new_terminal: terminal) -> void:
	super.remove_connected_terminal(new_terminal)
	update_accepts_from_trains()

func update_accepts_from_trains() -> void:
	reset_accepts_train()
	for coords: Vector2i in connected_terminals:
		var obj: terminal = terminal_map.get_terminal(coords)
		if obj is fixed_hold:
			add_accepts(obj)

func add_accepts(obj: terminal) -> void:
	for index: int in terminal_map.get_number_of_goods():
		if obj.does_accept(index):
			add_accept(index)

func reset_accepts_train() -> void:
	reset_accepts()

func day_tick() -> void:
	distribute_cargo()
