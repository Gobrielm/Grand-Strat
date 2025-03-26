class_name recipe extends Node

var inputs = {}

var outputs = {}

func _init(new_inputs: Dictionary, new_outputs: Dictionary):
	inputs = new_inputs
	outputs = new_outputs

static var set_recipes = []

static func create_set_recipes():
	#Primary
	set_recipes.append([{}, {"clay" = 1}])
	set_recipes.append([{}, {"sand" = 1}])
	set_recipes.append([{}, {"sulfur" = 1}])
	set_recipes.append([{}, {"lead" = 1}])
	set_recipes.append([{}, {"iron" = 1}])
	set_recipes.append([{}, {"coal" = 1}])
	set_recipes.append([{}, {"copper" = 1}])
	set_recipes.append([{}, {"zinc" = 1}])
	
	set_recipes.append([{}, {"wood" = 1}])
	set_recipes.append([{}, {"salt" = 1}])
	set_recipes.append([{}, {"grain" = 1}])
	set_recipes.append([{}, {"livestock" = 1}])
	set_recipes.append([{}, {"fish" = 1}])
	set_recipes.append([{}, {"fruit" = 1}])
	set_recipes.append([{}, {"cotton" = 1}])
	set_recipes.append([{}, {"silk" = 1}])
	
	set_recipes.append([{}, {"spices" = 1}])
	set_recipes.append([{}, {"coffee" = 1}])
	set_recipes.append([{}, {"tea" = 1}])
	set_recipes.append([{}, {"tobacco" = 1}])
	set_recipes.append([{}, {"gold" = 1}])
	#Secondary
	set_recipes.append([{"wood" = 3}, {"lumber" = 1}])
	set_recipes.append([{"wood" = 2}, {"paper" = 1}])
	set_recipes.append([{"lumber" = 3}, {"furniture" = 1}])
	set_recipes.append([{"lumber" = 4}, {"wagons" = 1}])
	set_recipes.append([{"lumber" = 10}, {"boats" = 1}])

static func get_set_recipes() -> Array:
	return set_recipes
