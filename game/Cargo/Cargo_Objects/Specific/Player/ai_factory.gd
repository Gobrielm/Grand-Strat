class_name ai_factory extends player_factory

func change_orders() -> void:
	change_buy_orders()
	change_sell_orders()

func change_buy_orders() -> void:
	for type: int in inputs:
		change_order(type, inputs[type])

func change_sell_orders() -> void:
	for type: int in outputs:
		change_order(type, outputs[type])

func change_order(type: int, base_amount: int) -> void:
	var price: float = local_pricer.get_local_price(type)
	var norm: float = local_pricer.get_base_price(type)
	#TODO; Add logic about setting default order to the output amountr
	if norm * 1.5 < price and randi() % 10 == 0:
		var order: trade_order = get_order(type)
		order.change_amount(order.get_amount() + 1)
	elif norm * 0.75 > price and randi() % 10 == 0:
		var order: trade_order = get_order(type)
		order.change_amount(order.get_amount() - 1)
	else:
		var order: trade_order = get_order(type)
		if order == null:
			place_order(type, base_amount, false)
		else:
			order.change_amount(base_amount)

func day_tick() -> void:
	change_orders()
	super.day_tick()
