class_name recipe extends RefCounted

var inputs: Dictionary = {}

var outputs: Dictionary = {}

func _init(new_inputs: Dictionary, new_outputs: Dictionary) -> void:
	inputs = new_inputs
	outputs = new_outputs

static var set_recipes: Array = []

static func create_set_recipes() -> void:
	#Primary
	add_set_recipe([{}, {"clay" = 1}])
	add_set_recipe([{}, {"sand" = 1}])
	add_set_recipe([{}, {"sulfur" = 1}])
	add_set_recipe([{}, {"lead" = 1}])
	add_set_recipe([{}, {"iron" = 1}])
	add_set_recipe([{}, {"coal" = 1}])
	add_set_recipe([{}, {"copper" = 1}])
	add_set_recipe([{}, {"zinc" = 1}])
	
	add_set_recipe([{}, {"wood" = 1}])
	add_set_recipe([{}, {"salt" = 1}])
	add_set_recipe([{}, {"grain" = 1}])
	add_set_recipe([{}, {"livestock" = 1}])
	add_set_recipe([{}, {"fish" = 1}])
	add_set_recipe([{}, {"fruit" = 1}])
	add_set_recipe([{}, {"cotton" = 1}])
	add_set_recipe([{}, {"silk" = 1}])
	
	add_set_recipe([{}, {"spices" = 1}])
	add_set_recipe([{}, {"coffee" = 1}])
	add_set_recipe([{}, {"tea" = 1}])
	add_set_recipe([{}, {"tobacco" = 1}])
	add_set_recipe([{}, {"gold" = 1}])
	#Secondary
	add_set_recipe([{"wood" = 3}, {"lumber" = 1}])
	add_set_recipe([{"wood" = 2}, {"paper" = 1}])
	add_set_recipe([{"lumber" = 3}, {"furniture" = 1}])
	add_set_recipe([{"lumber" = 4}, {"wagons" = 1}])
	add_set_recipe([{"lumber" = 10}, {"boats" = 1}])

static func add_set_recipe(readable_recipe: Array) -> void:
	var input: Dictionary = {}
	var output: Dictionary = {}
	for i: String in readable_recipe[0]:
		input[CargoInfo.get_instance().get_cargo_type(i)] = readable_recipe[0][i]
	for i: String in readable_recipe[1]:
		output[CargoInfo.get_instance().get_cargo_type(i)] = readable_recipe[1][i]
	set_recipes.append([input, output])

static func get_set_recipes() -> Array:
	return set_recipes
