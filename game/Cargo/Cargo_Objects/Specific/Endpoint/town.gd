class_name town extends ai_factory

var internal_factories: Dictionary[int, Array] = {}

func _init(new_location: Vector2i, _player_id: int, mult: int = 1) -> void:
	var dict: Dictionary = create_inputs(mult)
	super._init(new_location, _player_id, dict, {})
	level = mult

func create_inputs(mult: int) -> Dictionary:
	var toReturn: Dictionary = {}
	#TODO: Redo scaling of goods needed
	toReturn[terminal_map.get_cargo_type("grain")] = 1 * mult
	toReturn[terminal_map.get_cargo_type("wood")] = 0.5 * mult
	toReturn[terminal_map.get_cargo_type("wine")] = 0.2 * mult
	toReturn[terminal_map.get_cargo_type("furniture")] = 0.75 * mult
	toReturn[terminal_map.get_cargo_type("wagons")] = 0.1 * mult
	toReturn[terminal_map.get_cargo_type("paper")] = 0.3 * mult
	toReturn[terminal_map.get_cargo_type("lanterns")] = 0.3 * mult
	toReturn[terminal_map.get_cargo_type("luxury_clothes")] = 0.2 * mult
	toReturn[terminal_map.get_cargo_type("clothes")] = 0.75 * mult
	toReturn[terminal_map.get_cargo_type("bread")] = 0.5 * mult
	toReturn[terminal_map.get_cargo_type("meat")] = 0.3 * mult
	toReturn[terminal_map.get_cargo_type("liquor")] = 0.2 * mult
	toReturn[terminal_map.get_cargo_type("coffee")] = 0.2 * mult
	toReturn[terminal_map.get_cargo_type("tea")] = 0.2 * mult
	toReturn[terminal_map.get_cargo_type("porcelain")] = 0.05 * mult
	toReturn[terminal_map.get_cargo_type("cigarettes")] = 0.05 * mult
	toReturn[terminal_map.get_cargo_type("gold")] = 0.01 * mult
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

func update_level() -> void:
	var map_data_obj: map_data = map_data.get_instance()
	var new_level: int = map_data_obj.get_population_as_level(map_data_obj.get_province_id(location))
	if level != new_level:
		level = new_level
		inputs = create_inputs(level)

func day_tick() -> void:
	withdraw()
	if trade_orders.size() != 0:
		distribute_cargo()

func month_tick() -> void:
	update_level()
	change_orders()
	add_pop_money()
	super.month_tick()
