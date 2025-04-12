class_name broker extends fixed_hold

var trade_orders: Dictionary[int, trade_order] = {}

var connected_terminals: Dictionary[Vector2i, int] = {}

const MAX_SUPPLY_DISTANCE: int = 5

func can_afford(price: int) -> bool:
	return cash >= price

func get_desired_cargo_to_load(type: int, price_per: float) -> int:
	if trade_orders.has(type):
		var trade_order_obj: trade_order = trade_orders[type]
		if trade_order_obj.is_buy_order():
			return min(max_amount - get_cargo_amount(type), get_amount_can_buy(price_per), trade_order_obj.get_amount())
	return 0

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

func add_connected_terminal(new_terminal: terminal, distance: int) -> void:
	connected_terminals[new_terminal.get_location()] = distance

func remove_connected_terminal(new_terminal: terminal) -> void:
	connected_terminals.erase(new_terminal.get_location())

func get_ordered_connected_terms() -> Array[terminal]:
	var toReturn: Array[terminal]
	var stack: sorted_stack = sorted_stack.new()
	#TODO: Add checking price to order
	for tile: Vector2i in connected_terminals:
		var dist: int = connected_terminals[tile]
		stack.insert_element(tile, dist)
	#Casts to Array[terminal]
	toReturn = stack.get_array_of_elements()
	return toReturn

func get_randomized_connected_terms() -> Array[terminal]:
	var toReturn: Array[terminal] = []
	for tile: Vector2i in connected_terminals:
		toReturn.push_back(terminal_map.get_terminal(tile))
	toReturn.shuffle()
	return toReturn
