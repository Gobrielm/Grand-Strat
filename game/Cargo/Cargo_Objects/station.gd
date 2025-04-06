class_name station extends fixed_hold

var connected_terminals: Dictionary = {}

var trade_orders: Dictionary = {}

func _init(new_location: Vector2i, _player_owner: int) -> void:
	super._init(new_location, _player_owner)

func get_orders() -> Dictionary:
	return trade_orders

func place_order(type: int, amount: int, buy: bool) -> void:
	var fact: factory_template
	for term: terminal in connected_terminals.values():
		if term is factory_template and ((term.does_create(type) and buy) or (term.does_accept(type) and !buy)):
			fact = term
	var order: trade_order
	if fact != null:
		order = trade_order.new(type, amount, buy, fact.get_location())
		fact.add_order(location, order)
	else:
		order = trade_order.new(type, amount, buy, location)
	trade_orders[type] = order

func edit_order(type: int, amount: int, buy: bool) -> void:
	if trade_orders.has(type):
		#Test that order changes on both sides
		var order: trade_order = trade_orders[type]
		order.change_buy(buy)
		order.change_amount(amount)
	else:
		place_order(type, amount, buy)

func remove_order(type: int) -> void:
	if trade_orders.has(type):
		var order: trade_order = trade_orders[type]
		var term_coords: Vector2i = order.get_coords_of_factory()
		if term_coords != location:
			var term: terminal = connected_terminals[term_coords]
			term.remove_order(location, type)
		trade_orders.erase(type)
		order.queue_free()

func can_take_type(type: int, term: terminal) -> bool:
	if term is factory or term is apex_factory:
		return term.does_accept(type)
	return false

func get_desired_cargo_to_load(type: int, price_per: float) -> int:
	return min(max_amount - get_cargo_amount(type), get_amount_can_buy(price_per))

func can_afford(price: int) -> bool:
	return cash >= price

func buy_cargo(type: int, amount: int, price_per: float) -> void:
	add_cargo_ignore_accepts(type, amount)
	remove_cash(round(amount * price_per))

func distribute_cargo() -> void:
	var array: Array = randomize_trade_orders()
	for order: trade_order in array:
		complete_order(order)

func randomize_trade_orders() -> Array:
	var choices: Array = []
	var toReturn: Array = []
	for order: trade_order in trade_orders.values():
		if order.is_sell_order():
			choices.append(order)
	while !choices.is_empty():
		var rand_num: int = randi_range(0, choices.size() - 1)
		var choice: trade_order = choices.pop_at(rand_num)
		toReturn.append(choice)
	return toReturn

func complete_order(order: trade_order) -> void:
	var type: int = order.get_type()
	if order.get_coords_of_factory() == location:
		return
	var fact: factory_template = terminal_map.get_terminal(order.get_coords_of_factory())
	var amount: int = min(fact.get_desired_cargo_to_load(type), order.get_amount(), LOAD_TICK_AMOUNT)
	amount = transfer_cargo(type, amount)
	assert(amount > -100000)
	var price: float = fact.get_local_price(type)
	fact.buy_cargo(type, amount)
	add_cash(round(amount * price))

func add_connected_terminal(new_terminal: terminal) -> void:
	connected_terminals[new_terminal.get_location()] = new_terminal
	update_accepts_from_trains()

func remove_connected_terminal(new_terminal: terminal) -> void:
	connected_terminals.erase(new_terminal.get_location())
	update_accepts_from_trains()

func update_accepts_from_trains() -> void:
	reset_accepts_train()
	for obj:terminal in connected_terminals.values():
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
