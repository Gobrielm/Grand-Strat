class_name broker extends fixed_hold

var trade_orders: Dictionary[int, trade_order] = {}

var connected_terminals: Dictionary[Vector2i, int] = {}

const MAX_SUPPLY_DISTANCE: int = 5

var local_pricer: local_price_controller

func can_afford(price: int) -> bool:
	return cash >= price

func get_local_prices() -> Dictionary:
	return local_pricer.local_prices

func get_local_price(type: int) -> float:
	return local_pricer.get_local_price(type)

func get_desired_cargo_to_load(type: int, price_per: float) -> int:
	if trade_orders.has(type):
		var trade_order_obj: trade_order = trade_orders[type]
		if trade_order_obj.is_buy_order() and is_price_acceptable(type, price_per):
			return min(max_amount - get_cargo_amount(type), get_amount_can_buy(price_per), trade_order_obj.get_amount())
	return 0

#Assuming they are buying
func is_price_acceptable(type: int, price_per: float) -> bool:
	if get_local_price(type) < price_per * 0.8:
		return false
	return true

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

func get_directly_connected_terms(type: int) -> Array[broker]:
	return get_connected_terms_within_dist(type, 1)

func get_connected_terms(type: int) -> Array[broker]:
	return get_connected_terms_within_dist(type)

func get_connected_terms_within_dist(type: int, max_dist: int = 100) -> Array[broker]:
	var stack: sorted_stack = sorted_stack.new()
	
	for tile: Vector2i in connected_terminals:
		if connected_terminals[tile] < max_dist:
			var broker_obj: broker = terminal_map.get_broker(tile)
			if broker_obj != null and broker_obj.get_order(type) != null:
				stack.insert_element(terminal_map.get_broker(tile), broker_obj.get_local_price(type))
	
	var toReturn: Array[broker] = []
	for element: broker in stack.get_array_of_elements():
		toReturn.append(element)
	return toReturn
