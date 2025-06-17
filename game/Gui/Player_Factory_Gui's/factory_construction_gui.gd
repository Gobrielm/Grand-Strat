extends Window

var current_coords: Vector2i
var current_recipe: Array
var current_materials: Dictionary
var needed_materials: Dictionary

const time_every_update: int = 1
var progress: float = 0.0

@onready var current_recipe_textbox: Label = $Control/Current_Recipe
@onready var material_list: ItemList = $Control/Material_List

func _process(delta: float) -> void:
	progress += delta
	if progress > time_every_update:
		progress = 0
		refresh_window()

func _ready() -> void:
	hide()

func open_window(coords: Vector2i) -> void:
	popup()
	current_coords = coords
	refresh_window()

func refresh_window() -> void:
	if current_coords != Vector2i(0, 0):
		current_recipe = []
		current_materials = {}
		needed_materials = {}
		request_recipe.rpc_id(1, current_coords)
		request_construction_materials.rpc_id(1, current_coords)

func _on_close_requested() -> void:
	current_coords = Vector2i(0, 0)
	hide()

@rpc("any_peer", "call_local", "unreliable")
func request_recipe(coords: Vector2i) -> void:
	var recipe_item: Array = TerminalMap.get_instance().get_construction_site_recipe(coords)
	add_recipe.rpc_id(multiplayer.get_remote_sender_id(), recipe_item)

@rpc("any_peer", "call_local", "unreliable")
func request_construction_materials(coords: Vector2i) -> void:
	var new_needed_materials: Dictionary = TerminalMap.get_instance().get_construction_materials(coords)
	var new_current_materials: Dictionary = TerminalMap.get_instance().get_cargo_dict(coords)
	var prices: Dictionary = TerminalMap.get_instance().get_local_prices(coords)
	set_construction_materials.rpc_id(multiplayer.get_remote_sender_id(), new_current_materials, new_needed_materials, prices)

@rpc("authority", "call_local", "unreliable")
func add_recipe(recipe_item: Array) -> void:
	current_recipe = recipe_item
	current_recipe_textbox.text = get_name_for_recipe(current_recipe[0], current_recipe[1])

@rpc("authority", "call_local", "unreliable")
func set_construction_materials(new_current_materials: Dictionary, new_needed_materials: Dictionary, prices: Dictionary) -> void:
	current_materials = new_current_materials
	needed_materials = new_needed_materials
	material_list.clear()
	for type: int in needed_materials:
		var price: float = Utils.round(prices[type], 2)
		material_list.add_item(CargoInfo.get_instance().get_cargo_name(type) + " - $" + str(price) + " " + str(current_materials[type]) + "/" + str(needed_materials[type]))

func get_name_for_recipe(inputs: Dictionary, outputs: Dictionary) -> String:
	var toReturn: String = ""
	for type: int in inputs:
		toReturn += CargoInfo.get_instance().get_cargo_name(type) + " "
		toReturn += str(inputs[type]) + "+ "
	toReturn = toReturn.left(toReturn.length() - 2)
	toReturn += " = "
	for type: int in outputs:
		toReturn += CargoInfo.get_instance().get_cargo_name(type) + " "
		toReturn += str(outputs[type]) + "+ "
	toReturn = toReturn.left(toReturn.length() - 2)
	return toReturn

func _on_wipe_recipe_pressed() -> void:
	request_destory_recipe.rpc_id(1, current_coords)
	hide()

@rpc("any_peer", "call_local", "reliable")
func request_destory_recipe(coords: Vector2i) -> void:
	TerminalMap.get_instance().destory_recipe(coords)
