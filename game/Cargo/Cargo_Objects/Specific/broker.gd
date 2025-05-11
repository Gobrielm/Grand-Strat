class_name broker extends fixed_hold

var trade_orders: Dictionary[int, trade_order] = {}

var connected_terminals: Dictionary[Vector2i, bool] = {} #Set of directly connected terminals

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

func place_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	var order: trade_order = trade_order.new(type, amount, buy, max_price)
	trade_orders[type] = order

func edit_order(type: int, amount: int, buy: bool, max_price: float) -> void:
	if trade_orders.has(type):
		#Test that order changes on both sides
		var order: trade_order = trade_orders[type]
		order.change_buy(buy)
		order.set_max_price(max_price)
		order.change_amount(amount)
	else:
		place_order(type, amount, buy, max_price)

func get_order(type: int) -> trade_order:
	if trade_orders.has(type):
		return trade_orders[type]
	return null

func get_orders() -> Dictionary:
	return trade_orders

func remove_order(type: int) -> void:
	if trade_orders.has(type):
		var order: trade_order = trade_orders[type]
		trade_orders.erase(type)

func add_connected_terminal(new_terminal: terminal) -> void:
	connected_terminals[new_terminal.get_location()] = true

func remove_connected_terminal(new_terminal: terminal) -> void:
	connected_terminals.erase(new_terminal.get_location())

func distribute_cargo() -> void:
	assert(false, "Default Implementation")

func distribute_from_order(order: trade_order) -> void:
	for coords: Vector2i in connected_terminals:
		var broker_obj: broker = terminal_map.get_broker(coords)
		if broker_obj.does_accept(order.get_type()):
			distribute_to_order(broker_obj, order)

func distribute_to_order(_broker: broker, order: trade_order) -> void:
	var type: int = order.get_type()
	#Price is an average between buyer and seller
	var price1: float = get_local_price(type)
	var price2: float = _broker.get_local_price(type)
	var price: float = max(price1, price2) - (abs(price1 - price2) / 2)
	if !order.price_is_acceptable(price):
		return
	var desired: int = _broker.get_desired_cargo_to_load(type, price)
	var amount: int = min(desired, order.get_amount())
	local_pricer.report_attempt(type, desired)
	if amount > 0:
		amount = transfer_cargo(type, amount)
		_broker.buy_cargo(type, amount, price)
		add_cash(round(amount * price))
