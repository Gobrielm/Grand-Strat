class_name organization extends Node

var supply_needed: Dictionary[int, int] #Cargo type -> amount
var current_supply: fixed_hold

func _init(p_supply_needed: Dictionary[int, int], p_owner: int) -> void:
	supply_needed = p_supply_needed
	current_supply = fixed_hold.new(Vector2i(0, 0), p_owner)
	for type: int in supply_needed:
		current_supply.add_accept(type)

func get_organization() -> float:
	var fulfillment: float = 0.0
	
	var total_variety_of_goods: int = supply_needed.size()
	var max_penalty_per_type: float = 100.0 / total_variety_of_goods
	
	for type: int in supply_needed:
		var fulfillment_of_type: float = min(current_supply.get_cargo_amount(type) / float(supply_needed[type]), 1.0)
		fulfillment += max_penalty_per_type * fulfillment_of_type
	
	return fulfillment

#Return amount added
func add_cargo(type: int, amount: int) -> int:
	return current_supply.add_cargo(type, amount)

func get_desired_cargo(type: int) -> int:
	return current_supply.get_desired_cargo(type)
