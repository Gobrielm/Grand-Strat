class_name recipe extends Node

var inputs = {}

var outputs = {}

func _init(new_inputs: Dictionary, new_outputs: Dictionary):
	inputs = new_inputs
	outputs = new_outputs

static var set_recipes = []

static func create_set_recipes():
	set_recipes.append([{"wood" = 3}, {"lumber" = 1}])
	set_recipes.append([{"wood" = 2}, {"paper" = 1}])
	set_recipes.append([{"lumber" = 3}, {"furniture" = 1}])
	set_recipes.append([{"lumber" = 4}, {"wagons" = 1}])
	set_recipes.append([{"lumber" = 10}, {"boats" = 1}])

static func get_set_recipes() -> Array:
	return set_recipes
