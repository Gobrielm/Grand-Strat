class_name base_pop extends RefCounted

static var PEOPLE_PER_POP: int = 1000
static var total_pops: int = 0
static var base_needs: Dictionary[int, float]  #Necessities for every pop
static var specialities: Dictionary[int, float] = {} #Specialities, different for each type of pop
const INITIAL_WEALTH: int = 1000

var pop_id: int
var education_level: int = 0
var wealth: float = INITIAL_WEALTH
var home_prov_id: int = -1
var culture: Object = null #TODO
var income: float = 0.0

static func create_base_needs() -> void:
	base_needs = {
		terminal_map.get_instance().get_cargo_type("grain"): 1, terminal_map.get_instance().get_cargo_type("wood"): 0.3,
		terminal_map.get_instance().get_cargo_type("salt"): 0.1, terminal_map.get_instance().get_cargo_type("fish"): 0.2,
		terminal_map.get_instance().get_cargo_type("fruit"): 0.2, terminal_map.get_instance().get_cargo_type("meat"): 0.2,
		terminal_map.get_instance().get_cargo_type("bread"): 0.3, terminal_map.get_instance().get_cargo_type("clothes"): 0.3,
		terminal_map.get_instance().get_cargo_type("furniture"): 0.3
	}

func _init(home_prov: int, p_culture: Variant) -> void:
	home_prov_id = home_prov
	culture = p_culture
	pop_id = total_pops
	total_pops += 1

# === Employment === 
func find_employment() -> void:
	pass
	#var prov: province = map_data.get_instance().get_province(home_prov_id)
	#var work: factory_template = prov.find_employment(self)
	#work.work_here(self)

func is_seeking_employement() -> bool:
	if income == 0:
		return true
	else:
		#TODO: Stuff with expected sol
		return false

func pay_wage(wage: float) -> void:
	wealth += wage

func employ(wage: float) -> void:
	income = wage

func fire() -> void:
	income = 0.0

# === Living Standards === 
func get_income() -> float:
	return income

func is_income_acceptable(p_income: float) -> bool:
	return p_income > get_expected_income() and p_income > get_income()

func get_expected_income() -> float:
	#TODO: Stuff with expected sol and province sol
	return 0.0

func get_sol() -> float:
	return get_income()

func get_desired(type: int, price: float) -> float:
	var amount: float = 0
	if base_needs.has(type):
		amount += base_needs[type]
	if specialities.has(type):
		amount += specialities[type]
	if amount * price < wealth:
		return amount
	return 0

func buy_good(amount: float, price: float) -> void:
	wealth -= amount * price
	if wealth < 0:
		#TODO, uh-oh
		assert(false)

# === Education === 
func get_education_level() -> int:
	return education_level

# === Wealth ===
func get_wealth() -> float:
	return wealth

func transfer_wealth() -> float:
	#Transfers 50% of wealth
	var toReturn: float = get_wealth() * 0.5
	wealth -= toReturn
	return toReturn
