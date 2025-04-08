class_name broker extends fixed_hold

var trade_orders: Dictionary = {}

const MAX_SUPPLY_DISTANCE: int = 5

func can_afford(price: int) -> bool:
	return cash >= price

func buy_cargo(type: int, amount: int, price_per: float) -> void:
	if amount == 0:
		return
	add_cargo_ignore_accepts(type, amount)
	remove_cash(round(amount * price_per))

func place_order(type: int, amount: int, buy: bool) -> void:
	var order: trade_order = trade_order.new(type, amount, buy)
	trade_orders[type] = order

func edit_order(type: int, amount: int, buy: bool) -> void:
	if trade_orders.has(type):
		#Test that order changes on both sides
		var order: trade_order = trade_orders[type]
		order.change_buy(buy)
		order.change_amount(amount)
	else:
		place_order(type, amount, buy)

func get_order(type: int) -> trade_order:
	if trade_orders.has(type):
		return trade_orders[type]
	return null

func remove_order(type: int) -> void:
	if trade_orders.has(type):
		var order: trade_order = trade_orders[type]
		trade_orders.erase(type)
		order.queue_free()

func get_road_supplied_terminals() -> Array[terminal]:
	var visited: Dictionary = {}
	visited[location] = 0
	var world_map: TileMapLayer = Utils.world_map
	var queue: Array = [location]
	var toReturn: Array[terminal] = []
	while !queue.is_empty():
		var curr: Vector2i = queue.pop_front()
		
		for tile: Vector2i in world_map.get_surrounding_cells(curr):
			if !Utils.is_tile_water(tile):
				if !visited.has(tile) or visited[tile] > visited[curr] + 1:
					visited[tile] = visited[curr] + 1
					if visited[tile] < MAX_SUPPLY_DISTANCE:
						queue.push_back(tile)
				#TODO: Ensure this includes everything that needs trade
				if terminal_map.is_broker(tile) or terminal_map.is_station(tile):
					toReturn.push_back(terminal_map.get_terminal(tile))
				
	return toReturn
