class_name factory_template extends broker

const COST_FOR_UPGRADE: int = 1000

var income_array: Array[float]
var last_money_cash: float

var level: int
var employees: Array[base_pop] = []
var employment_total: int

var inputs: Dictionary
var outputs: Dictionary

const DEFAULT_BATCH_SIZE: int = 1

func _init(new_location: Vector2i, _player_owner: int, new_inputs: Dictionary, new_outputs: Dictionary) -> void:
	super._init(new_location, _player_owner)
	inputs = new_inputs
	outputs = new_outputs
	local_pricer = local_price_controller.new(inputs, outputs)
	level = 1
	employment_total = 1000

# === Trade ===

#For Selling only, assuming only one type to sell
func get_min_price(_type: int) -> float:
	assert(outputs.size() == 1)
	assert(inputs.size() == 0)
	#TODO: ventually use expenses
	return 0.0

#For Buying only, assuming end-node
func get_max_price(_type: int) -> float:
	assert(outputs.size() == 0)
	#TODO: eventually use expenses
	return 100.0

func get_monthly_demand(type: int) -> int:
	return inputs[type] * clock_singleton.get_instance().get_days_in_current_month() * level

func get_monthly_supply(type: int) -> int:
	return outputs[type] * clock_singleton.get_instance().get_days_in_current_month() * level

# === Creation of Goods ===

func does_create(type: int) -> bool:
	return outputs.has(type)

func create_recipe() -> void:
	var batch_size: int = get_batch_size()
	remove_inputs(batch_size)
	add_outputs(batch_size)

func get_batch_size() -> int:
	var batch_size: int = get_level()
	for index: int in inputs:
		var amount: int = inputs[index]
		batch_size = min(floor(storage[index] / amount), batch_size)
	for index: int in outputs:
		var amount: int = outputs[index]
		batch_size = min(floor((max_amount - storage[index]) / amount), batch_size)
	return batch_size

func remove_inputs(batch_size: int) -> void:
	for index: int in inputs:
		var amount: int = inputs[index] * batch_size
		storage[index] -= amount

func add_outputs(batch_size: int) -> void:
	for index: int in outputs:
		var amount: int = outputs[index] * batch_size
		amount = add_cargo_ignore_accepts(index, amount)

# === Selling ===

func distribute_cargo() -> void:
	for type: int in outputs:
		if trade_orders.has(type):
			var order: trade_order = trade_orders[type]
			if order.is_sell_order():
				distribute_from_order(order)

# === Levels & Upgrades ===

func get_level() -> int:
	#TODO: Employment
	#var employment: int = get_employement()
	#if employment == 0:
		#return 0
	#return round(level * employment / employment_total)
	return level

func get_cost_for_upgrade() -> int:
	return COST_FOR_UPGRADE

func upgrade() -> bool:
	if inputs.is_empty() and outputs.size() == 1:
		var cargo_values: Node = Utils.cargo_values
		var mag: int = cargo_values.get_tile_magnitude(location, outputs.values()[0])
		if mag > level:
			var cost: int = get_cost_for_upgrade()
			if cash >= cost:
				remove_cash(cost)
				level += 1
				employment_total = level * 1000
				return true
	return false

func admin_upgrade() -> void:
	if inputs.is_empty() and outputs.size() == 1:
		var cargo_values: Node = Utils.cargo_values
		var mag: int = cargo_values.get_tile_magnitude(location, outputs.values()[0])
		if mag > level:
			level += 1
			employment_total = level * 1000

func update_income_array() -> void:
	var income: float = cash - last_money_cash
	income_array.push_front(income)
	if income_array.size() == 27: #Last two years
		income_array.pop_back()
	last_money_cash = cash

func get_last_month_income() -> float:
	if income_array.size() == 0:
		return 0
	return income_array[0]

# === Employment ===
func get_employement() -> int:
	return employees.size() * base_pop.PEOPLE_PER_POP

func is_hiring() -> bool:
	return employment_total - get_employement() >= base_pop.PEOPLE_PER_POP and get_last_month_income() * 0.9 > 0

func is_firing() -> bool:
	return get_employement() != 0 and get_last_month_income() < 0

func get_wage() -> float:
	#TODO: Kinda stupid but testing
	var available_for_wages: float = get_last_month_income() * 0.9
	var wage: float = available_for_wages / employment_total
	return wage

func work_here(pop: base_pop) -> void:
	if employment_total - employment_total >= base_pop.PEOPLE_PER_POP:
		employees.insert(randi() % employees.size(), pop) #Randomly insert
		pop.employ(self)

func pay_employees() -> void:
	var wage: float = get_wage()
	for employee: base_pop in employees:
		employee.pay_wage(transfer_cash(wage))

func fire_employees() -> void:
	var fired: int = 0
	var to_fire: int = max(employees.size() * 0.1, 100)
	while true:
		var rand_index: int = randi() % employees.size()
		var pop: base_pop = employees.pop_at(rand_index)
		pop.fire(self)
		fired += 1
		if fired >= to_fire:
			break

# === Processes ===

func day_tick() -> void:
	print("Default implementation")
	assert(false)

func month_tick() -> void:
	update_income_array()
	pay_employees()
	if is_firing():
		fire_employees()
	
