class_name town extends ai_factory

var internal_factories: Dictionary[int, Array] = {} # Owner id -> Array[factory_templates]
var city_pops: Dictionary[int, city_pop] = {} #Pop id -> pop
var cash: float = 0.0 #Cash available to buy things

func _init(new_location: Vector2i) -> void:
	super._init(new_location, 0, {}, {}) #Inits with 0 pops, and player_id = 0

func add_cash(amount: float) -> void:
	money_controller.get_instance().add_money_to_player(player_owner, amount)

func remove_cash(amount: float) -> void:
	money_controller.get_instance().remove_money_from_player(player_owner, amount)

func get_cash() -> float:
	return money_controller.get_instance().get_money(player_owner)

func add_pop_to_inputs(new_inputs: Dictionary[int, float], pop: city_pop) -> void:
	if pop.get_income() > 0 or pop.get_wealth() > 0:
		#Essentials
		new_inputs[terminal_map.get_cargo_type("grain")] += 1
		new_inputs[terminal_map.get_cargo_type("wood")] += 0.5
		new_inputs[terminal_map.get_cargo_type("bread")] += 0.3
		new_inputs[terminal_map.get_cargo_type("clothes")] += 0.3
		new_inputs[terminal_map.get_cargo_type("furniture")] += 0.3
		new_inputs[terminal_map.get_cargo_type("meat")] += 0.3
		new_inputs[terminal_map.get_cargo_type("liquor")] += 0.3
		new_inputs[terminal_map.get_cargo_type("tobacco")] += 0.1
	if pop.get_income() > 10:
		#Specialty
		new_inputs[terminal_map.get_cargo_type("wagons")] += 0.1
		new_inputs[terminal_map.get_cargo_type("paper")] += 0.3
		new_inputs[terminal_map.get_cargo_type("lanterns")] += 0.3
	
	if pop.get_income() > 20 and pop.get_wealth() > 200:
		#Luxuries
		new_inputs[terminal_map.get_cargo_type("wine")] += 0.5
		new_inputs[terminal_map.get_cargo_type("luxury_clothes")] += 0.3
		new_inputs[terminal_map.get_cargo_type("coffee")] += 1
		new_inputs[terminal_map.get_cargo_type("tea")] += 1
		new_inputs[terminal_map.get_cargo_type("porcelain")] += 0.3
		new_inputs[terminal_map.get_cargo_type("gold")] += 0.5
	
	#TODO: Merchants will buy extra goods with the goal of selling to other stations/ports

func add_factory_to_inputs(new_inputs: Dictionary[int, float], fact: factory_template) -> void:
	for type: int in fact.inputs:
		pass
		#new_inputs[type] += fact.get_desired_cargo_to_load(type)

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

#Used to get goods into the town
func update_town_inputs() -> void:
	var pops: Array[city_pop] = get_pops()
	set_max_storage(pops.size())
	var new_inputs: Dictionary[int, float] = create_blank_inputs() #Using float for amount for accumulation, but will round before assignment
	for pop: city_pop in pops:
		add_pop_to_inputs(new_inputs, pop)
	for facts: Array in internal_factories.values():
		for industry: factory_template in facts:
			add_factory_to_inputs(new_inputs, industry)
	
	for type: int in new_inputs:
		inputs[type] = round(new_inputs[type])
		if new_inputs[type] == 0:
			inputs.erase(type)

func create_blank_inputs() -> Dictionary[int, float]:
	var toReturn: Dictionary[int, float] = {}
	for type: int in terminal_map.get_number_of_goods():
		toReturn[type] = 0.0
	return toReturn

func get_pops() -> Array[city_pop]:
	var toReturn: Array[city_pop]
	toReturn.assign(city_pops.values())
	return toReturn

func day_tick() -> void:
	withdraw()
	if trade_orders.size() != 0:
		distribute_cargo()

func month_tick() -> void:
	update_town_inputs()
	change_orders()
	add_pop_money()
	super.month_tick()
