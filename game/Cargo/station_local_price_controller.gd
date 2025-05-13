class_name station_local_price_controller extends local_price_controller

func _init() -> void:
	pass

func add_cargo_type(type: int, starting_price: float = base_prices[type]) -> void:
	local_prices[type] = starting_price
	reset_attempts(type)

func remove_cargo_type(type: int) -> void:
	local_prices.erase(type)
	attempts_to_trade.erase(type)

func add_cargo_from_factory(fact: factory_template) -> void:
	for type: int in fact.outputs:
		add_cargo_type(type)
