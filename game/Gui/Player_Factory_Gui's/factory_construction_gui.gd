extends Window

var current_coords: Vector2i
var current_recipe: Array
var current_materials: Dictionary
var needed_materials: Dictionary

@onready var current_recipe_textbox = $Control/Current_Recipe
@onready var material_list = $Control/Material_List

func _ready():
	hide()

func open_window(coords: Vector2i):
	popup()
	current_recipe = []
	current_materials = {}
	needed_materials = {}
	current_coords = coords
	request_recipe.rpc_id(1, current_coords)
	request_construction_materials.rpc_id(1, current_coords)

func _on_close_requested():
	hide()

@rpc("any_peer", "call_local", "unreliable")
func request_recipe(coords: Vector2i):
	var recipe_item: Array = terminal_map.get_construction_site_recipe(coords)
	add_recipe.rpc_id(multiplayer.get_remote_sender_id(), recipe_item)

@rpc("any_peer", "call_local", "unreliable")
func request_construction_materials(coords: Vector2i):
	var new_needed_materials: Dictionary = terminal_map.get_construction_materials(coords)
	var new_current_materials: Dictionary = terminal_map.get_hold(coords)
	set_construction_materials.rpc_id(multiplayer.get_remote_sender_id(), new_current_materials, new_needed_materials)

@rpc("authority", "call_local", "unreliable")
func add_recipe(recipe_item: Array):
	current_recipe = recipe_item
	current_recipe_textbox.text = get_name_for_recipe(current_recipe[0], current_recipe[1])

@rpc("authority", "call_local", "unreliable")
func set_construction_materials(new_current_materials: Dictionary, new_needed_materials: Dictionary):
	current_materials = new_current_materials
	needed_materials = new_needed_materials
	for type: int in needed_materials:
		material_list.add_item(terminal_map.get_cargo_name(type) + " " + str(current_materials[type]) + "/" + str(needed_materials[type]))

func get_name_for_recipe(inputs: Dictionary, outputs: Dictionary) -> String:
	var toReturn: String = ""
	for input: String in inputs:
		toReturn += str(inputs[input]) + " "
		toReturn += input + "+ "
	toReturn = toReturn.left(toReturn.length() - 2)
	toReturn += " = "
	for output: String in outputs:
		toReturn += str(outputs[output]) + " "
		toReturn += output + "+ "
	toReturn = toReturn.left(toReturn.length() - 2)
	return toReturn

func _on_wipe_recipe_pressed():
	request_destory_recipe.rpc_id(1, current_coords)
	hide()

@rpc("any_peer", "call_local", "reliable")
func request_destory_recipe(coords: Vector2i):
	terminal_map.destory_recipe(coords)
