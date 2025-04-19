class_name station extends broker

var max_prices: Dictionary[int, float] = {}

func _init(new_location: Vector2i, _player_owner: int) -> void:
	super._init(new_location, _player_owner)

func get_local_price(type: int) -> float:
	return max_prices[type]

func place_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	var order: trade_order = trade_order.new(type, amount, buy, max_price)
	max_prices[type] = max_price
	trade_orders[type] = order

func edit_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	if trade_orders.has(type):
		#Test that order changes on both sides
		var order: trade_order = trade_orders[type]
		order.change_buy(buy)
		order.change_amount(amount)
		order.set_max_price(max_price)
		max_prices[type] = max_price
	else:
		place_order(type, amount, buy, max_price)

func get_order(type: int) -> trade_order:
	if trade_orders.has(type):
		return trade_orders[type]
	return null

func get_orders_magnitude() -> int:
	var tot: int = 0
	for order: trade_order in trade_orders.values():
		tot += order.amount
	return tot

func remove_order(type: int) -> void:
	if trade_orders.has(type):
		var order: trade_order = trade_orders[type]
		trade_orders.erase(type)
		order.queue_free()
		max_prices.erase(type)

func distribute_cargo() -> void:
	for order: trade_order in trade_orders.values():
		if order.is_sell_order():
			distribute_from_order(order)

func add_connected_terminal(new_terminal: terminal, distance: int) -> void:
	super.add_connected_terminal(new_terminal, distance)
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
