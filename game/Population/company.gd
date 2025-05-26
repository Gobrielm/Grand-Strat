class_name company extends firm

var income_array: Array[float]
var last_money_cash: float

var level: int
var employees: Array[base_pop] = []
var pops_needed: int

var econ_ai_id: int

var investments: Array[factory_template] = []

func _init(p_location: Vector2i, initial_employees: Array[base_pop]) -> void:
	location = p_location
	for pop: base_pop in initial_employees:
		employees.push_back(pop)
		cash += pop.transfer_wealth()
	var country_id: int = get_country_id()
	econ_ai_id = ai_manager.get_instance().create_new_economy_ai(country_id)

# === Investment ===
func can_invest() -> bool:
	#TODO: Placeholder Amount
	return cash > 1000

func make_investment() -> void:
	money_controller.get_instance().add_money_to_player(econ_ai_id, 1000)

func investment_tick() -> void:
	if can_invest():
		make_investment()

# === Levels & Upgrades ===
func get_level() -> int:
	#TODO: Employment
	var employment: int = get_employement()
	if employment == 0:
		return 0
	@warning_ignore("integer_division")
	return ((employment) / pops_needed)

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
	return employees.size()

func is_hiring() -> bool:
	return pops_needed - get_employement() >= 0 and get_last_month_income() * 0.9 > 0

func is_firing() -> bool:
	return get_employement() != 0 and get_last_month_income() < 0

func get_wage() -> float:
	#TODO: Kinda stupid but testing
	var available_for_wages: float = get_last_month_income() * 0.9
	var wage: float = available_for_wages / pops_needed
	return wage

func work_here(pop: base_pop) -> void:
	if is_hiring():
		employees.insert(randi() % employees.size(), pop) #Randomly insert
		pop.employ(get_wage())

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
		pop.fire()
		fired += 1
		if fired >= to_fire:
			break

# === Utility ===
func get_country_id() -> int:
	return tile_ownership.get_instance().get_country_id(location)

func month_tick() -> void:
	update_income_array()
	pay_employees()
	if is_firing():
		fire_employees()
	investment_tick()
