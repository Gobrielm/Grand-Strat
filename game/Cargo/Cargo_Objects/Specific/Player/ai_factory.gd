class_name ai_factory extends player_factory

const CASH_NEEDED_MULTIPLIER: int = 5
const MAX_AMOUNT_WANTED: float = 0.75


func change_orders() -> void:
	#TODO: This is assigning orders of random goods
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
	#TODO; Add logic about setting default order to the output amountr
	var max_price: float = price * 1.2 if buy else price * 0.8
	if get_order(type) == null:
		#TODO: Test, 20% leeway
		place_order(type, amount, buy, max_price)
	
	var order: trade_order = get_order(type)
	
	#Sell more if a lot in storage only if it creates the cargo type
	if storage[type] > max_amount * MAX_AMOUNT_WANTED and outputs.has(type):
		amount = outputs[type] * 2
	
	order.change_amount(amount)

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
	if total_diff > -0.05 and get_cost_for_upgrade() < get_cash() * CASH_NEEDED_MULTIPLIER and randi() % 5 == 0:
		upgrade()

func consider_upgrade_secondary() -> void:
	pass

func day_tick() -> void:
	change_orders()
	super.day_tick()

func month_tick() -> void:
	super.month_tick()
	consider_upgrade()
