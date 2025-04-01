extends apex_factory

func _init(new_location: Vector2i, _player_id: int) -> void:
	var dict: Dictionary = create_inputs()
	super._init(new_location, _player_id, dict)

func create_inputs() -> Dictionary:
	var toReturn: Dictionary = {}
	toReturn[terminal_map.get_cargo_type("grain")] = 1
	toReturn[terminal_map.get_cargo_type("wood")] = 1
	toReturn[terminal_map.get_cargo_type("wine")] = 1
	toReturn[terminal_map.get_cargo_type("furniture")] = 1
	toReturn[terminal_map.get_cargo_type("wagons")] = 1
	toReturn[terminal_map.get_cargo_type("paper")] = 1
	toReturn[terminal_map.get_cargo_type("lumber")] = 1
	toReturn[terminal_map.get_cargo_type("lanterns")] = 1
	toReturn[terminal_map.get_cargo_type("luxury_clothes")] = 1
	toReturn[terminal_map.get_cargo_type("clothes")] = 1
	toReturn[terminal_map.get_cargo_type("bread")] = 1
	toReturn[terminal_map.get_cargo_type("meat")] = 1
	toReturn[terminal_map.get_cargo_type("liquor")] = 1
	toReturn[terminal_map.get_cargo_type("coffee")] = 1
	toReturn[terminal_map.get_cargo_type("tea")] = 1
	toReturn[terminal_map.get_cargo_type("porcelain")] = 1
	toReturn[terminal_map.get_cargo_type("cigarettes")] = 1
	toReturn[terminal_map.get_cargo_type("gold")] = 1
	return toReturn

func check_input(type: int) -> bool:
	return inputs[type] <= storage[type]

func remove_input(type: int) -> void:
	remove_cargo(type, inputs[type])

func get_fulfillment(type: int) -> float:
	return local_pricer.get_change(type) / inputs[type]

func get_town_wants() -> Array:
	return inputs.keys()

func withdraw() -> void:
	for type: int in inputs:
		if check_input(type):
			print(type)
			#TODO: Do something if type is available to be used
			remove_input(type)

func day_tick() -> void:
	withdraw()
	if trade_orders.size() != 0:
		distribute_cargo()

func month_tick() -> void:
	for type: int in inputs:
		local_pricer.vary_input_price(get_monthly_demand(type), type)
	for type: int in outputs:
		local_pricer.vary_output_price(type)
