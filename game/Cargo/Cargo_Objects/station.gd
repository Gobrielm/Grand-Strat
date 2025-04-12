class_name station extends broker

func _init(new_location: Vector2i, _player_owner: int) -> void:
	super._init(new_location, _player_owner)

func get_orders() -> Dictionary:
	return trade_orders

func distribute_cargo() -> void:
	for order: trade_order in trade_orders.values():
		if order.is_sell_order():
			complete_order(order)

func complete_order(order: trade_order) -> void:
	var type: int = order.get_type()
	while (amount_traded < min(order.get_amount(), LOAD_TICK_AMOUNT)):
		for tile: Vector2i in connected_terminals:
			var term: terminal = terminal_map.get_terminal(tile)
			var amount: int = min(term.get_desired_cargo_to_load(type), order.get_amount(), LOAD_TICK_AMOUNT)
			amount = transfer_cargo(type, amount)
			assert(amount > -100000)
			var price: float = term.get_local_price(type)
			term.buy_cargo(type, amount)
			add_cash(round(amount * price))
	amount_traded = 0

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
