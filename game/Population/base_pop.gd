class_name base_pop extends Node

static var PEOPLE_PER_POP: int = 10

var education_level: int = 0
var wealth: float = 0
var home_prov_id: int = -1
var culture: Object = null #TODO
var employment: factory_template = null #TODO, how they own things/businesses

func _init(home_prov: int, p_culture: Variant) -> void:
	home_prov_id = home_prov
	culture = p_culture

# === Employment === 
func find_employment() -> void:
	var prov: province = map_data.get_instance().get_province(home_prov_id)
	var work: factory_template = prov.find_employment(self)
	work.work_here(self)

func is_seeking_employement() -> bool:
	if employment == null:
		return true
	else:
		#TODO: Stuff with expected sol
		return false

func pay_wage(wage: float) -> void:
	wealth += wage

func employ(industry: factory_template) -> void:
	employment = industry

func fire(_industry: factory_template) -> void:
	employment = null

# === Living Standards === 
func get_income() -> float:
	if employment is factory_template:
		return employment.get_wage()
	return 0.0

func is_income_acceptable(p_income: float) -> bool:
	return p_income > get_expected_income() and p_income > get_income()

func get_expected_income() -> float:
	#TODO: Stuff with expected sol and province sol
	return 0.0

func get_sol() -> float:
	return get_income()

# === Education === 
func get_education_level() -> int:
	return education_level
