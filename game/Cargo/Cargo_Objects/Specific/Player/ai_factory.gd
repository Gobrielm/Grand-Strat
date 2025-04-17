class_name ai_factory extends player_factory

const CASH_NEEDED_MULTIPLIER: int = 5

func change_orders() -> void:
	change_buy_orders()
	change_sell_orders()

func change_buy_orders() -> void:
	for type: int in inputs:
		change_order(type, true)

func change_sell_orders() -> void:
	for type: int in outputs:
		change_order(type, false)

func change_order(type: int, buy: bool) -> void:
	var amount: int = inputs[type] if buy else outputs[type]
	var price: float = local_pricer.get_local_price(type)
	var norm: float = local_pricer.get_base_price(type)
	#TODO; Add logic about setting default order to the output amountr
	var order: trade_order = get_order(type)
	if norm * 1.5 < price and randi() % 10 == 0:
		amount = min(order.get_amount() - 1, 1)
	elif norm * 0.75 > price and randi() % 10 == 0:
		amount = order.get_amount() + 1
	
	
	var max_price: float = price * 1.2 if buy else price * 0.8
	if order == null:
		#TODO: Test, 20% leeway
		place_order(type, amount, buy, max_price)
	else:
		order.change_amount(amount)
		order.set_max_price(max_price)

func consider_upgrade() -> void:
	#Primary Industry
	if inputs.is_empty():
		consider_upgrade_primary()
	#Secondary Industry
	else:
		consider_upgrade_secondary()

func consider_upgrade_primary() -> void:
	var total_diff: float = 0.0
	var amount: int = 0
	for type: int in outputs:
		total_diff += local_pricer.get_percent_difference(type)
		amount += 1
	total_diff /= amount
	#TODO: Consider changing constant
	if total_diff > -0.05 and get_cost_for_upgrade() < cash * CASH_NEEDED_MULTIPLIER and randi() % 5 == 0:
		upgrade()

func consider_upgrade_secondary() -> void:
	pass

func day_tick() -> void:
	change_orders()
	super.day_tick()

func month_tick() -> void:
	super.month_tick()
	consider_upgrade()
