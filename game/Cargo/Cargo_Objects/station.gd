class_name station extends broker

var connected_terminals: Dictionary = {}

func _init(new_location: Vector2i, _player_owner: int) -> void:
	super._init(new_location, _player_owner)

func get_orders() -> Dictionary:
	return trade_orders

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
	var amount_traded: int
	while (amount_traded < min(order.get_amount(), LOAD_TICK_AMOUNT)):
		for term: terminal in get_road_supplied_terminals():
			var amount: int = min(term.get_desired_cargo_to_load(type), order.get_amount(), LOAD_TICK_AMOUNT)
			amount = transfer_cargo(type, amount)
			assert(amount > -100000)
			var price: float = term.get_local_price(type)
			term.buy_cargo(type, amount)
			add_cash(round(amount * price))

func add_connected_terminal(new_terminal: terminal) -> void:
	connected_terminals[new_terminal.get_location()] = new_terminal
	update_accepts_from_trains()

func remove_connected_terminal(new_terminal: terminal) -> void:
	connected_terminals.erase(new_terminal.get_location())
	update_accepts_from_trains()

func update_accepts_from_trains() -> void:
	reset_accepts_train()
	for obj: terminal in connected_terminals.values():
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
