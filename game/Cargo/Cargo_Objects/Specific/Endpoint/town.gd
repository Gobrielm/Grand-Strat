class_name town extends ai_factory

func _init(new_location: Vector2i, _player_id: int, mult: int = 1) -> void:
	var dict: Dictionary = create_inputs(mult)
	super._init(new_location, _player_id, dict, {})
	level = mult

func create_inputs(mult: int) -> Dictionary:
	var toReturn: Dictionary = {}
	#TODO: Redo scaling of goods needed
	toReturn[terminal_map.get_cargo_type("grain")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("wood")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("wine")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("furniture")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("wagons")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("paper")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("lumber")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("lanterns")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("luxury_clothes")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("clothes")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("bread")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("meat")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("liquor")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("coffee")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("tea")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("porcelain")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("cigarettes")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("gold")] = 1 * mult
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
			#TODO: Do something if type is available to be used
			remove_input(type)

func add_pop_money() -> void:
	add_cash(round(level * 100.0))

func day_tick() -> void:
	change_orders()
	withdraw()
	if trade_orders.size() != 0:
		distribute_cargo()

func month_tick() -> void:
	add_pop_money()
	super.month_tick()
