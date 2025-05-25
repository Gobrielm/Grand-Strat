class_name base_pop extends Node

static var PEOPLE_PER_POP: int = 10

var education_level: int
var income: float
var wealth: float
var home_prov_id: int
var culture: Variant #TODO
var employment: Variant #TODO, how they own things/businesses

func _on_month_tick_timeout() -> void:
	wealth += income

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

# === Living Standards === 

func is_income_acceptable(p_income: float) -> bool:
	return p_income > get_expected_income() and p_income > income

func get_expected_income() -> float:
	#TODO: Stuff with expected sol and province sol
	return 0.0

func get_sol() -> float:
	return income

# === Education === 
func get_education_level() -> int:
	return education_level
