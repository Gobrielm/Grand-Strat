class_name organization extends Node

var supply_needed: Dictionary[int, int] #Cargo type -> amount
var current_supply: fixed_hold

func _init(p_supply_needed: Dictionary[int, int]) -> void:
	supply_needed = p_supply_needed
	current_supply = fixed_hold.new(Vector2i(0, 0), 0) #Owner doesn;t matter
	current_supply.set_max_storage(10)
	for type: int in supply_needed:
		current_supply.add_accept(type)
		current_supply.add_cargo(type, current_supply.max_amount)

func convert_to_client_array() -> Array:
	return [supply_needed, current_supply.storage]

func get_organization() -> float:
	var fulfillment: float = 0.0
	
	var total_variety_of_goods: int = supply_needed.size()
	var max_penalty_per_type: float = 100.0 / total_variety_of_goods
	
	for type: int in supply_needed:
		#TODO: Add amount it should have more than it needs every tick, ie supply_needed[type] * 25
		var fulfillment_of_type: float = min(current_supply.get_cargo_amount(type) / float(supply_needed[type] * 5), 1.0)
		fulfillment += max_penalty_per_type * fulfillment_of_type
	
	return fulfillment

func use_supplies() -> void:
	for type: int in supply_needed:
		if current_supply.get_cargo_amount(type) < supply_needed[type]:
			#TODO: Do something if supplies run 0
			break
		current_supply.remove_cargo(type, supply_needed[type])

#Return amount added
func add_cargo(type: int, amount: int) -> int:
	print("Resupplied")
	return current_supply.add_cargo(type, amount)

func get_desired_cargo(type: int) -> int:
	return current_supply.get_desired_cargo(type)
