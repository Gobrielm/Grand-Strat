class_name construction_site extends factory_template

var construction_materials = {}
var max_amounts = {}

func _init(coords: Vector2i, _player_owner: int):
	super._init(coords, _player_owner, {}, {})
	max_amount = 0

func buy_cargo(type: int, amount: int):
	add_cargo(type, amount)

func add_cargo(type: int, amount: int):
	if does_accept(type):
		storage[type] += min(amount, max_amounts[type] - storage[type])

func get_desired_cargo_to_load(type: int) -> int:
	var price = get_local_price(type)
	assert(price > 0)
	var amount: int = get_amount_can_buy(get_local_price(type))
	return min(amount, max_amounts[type] - get_cargo_amount(type))

#Recipe Stuff
func set_recipe(selected_recipe: Array):
	inputs = selected_recipe[0]
	outputs = selected_recipe[1]
	create_construction_materials()

func destroy_recipe() -> void:
	inputs = {}
	outputs = {}

func get_recipe() -> Array:
	return [inputs, outputs]

func has_recipe() -> bool:
	return !inputs.is_empty() or !outputs.is_empty()

#Construction_materials
func create_construction_materials():
	create_construction_material(terminal_map.get_cargo_type("lumber"), 100)
	create_construction_material(terminal_map.get_cargo_type("steel"), 20)
	create_construction_material(terminal_map.get_cargo_type("iron"), 50)
	create_construction_material(terminal_map.get_cargo_type("glass"), 50)
	create_construction_material(terminal_map.get_cargo_type("tools"), 25)

func create_construction_material(type: int, amount: int):
	add_accept(type)
	max_amounts[type] = amount
	construction_materials[type] = 50

func get_construction_materials() -> Dictionary:
	return construction_materials

func is_finished_constructing() -> bool:
	for type in construction_materials:
		if storage[type] < construction_materials[type]:
			return false
	return true

func day_tick():
	pass

func month_tick():
	if is_finished_constructing():
		#Transfrom to player_factory
		pass
	
