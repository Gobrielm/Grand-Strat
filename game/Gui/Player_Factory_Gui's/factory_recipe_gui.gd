extends Window

@onready var recipes: ItemList = $Control/Recipes
@onready var filter: LineEdit = $Control/Search_Bar
var current_coords
var current_recipes
var primary: bool = true
var timer: Timer

func _input(event):
	if event.is_action_pressed("click"):
		if gui_get_hovered_control() != filter:
			filter.release_focus()
	elif event.is_action_pressed("deselect"):
		filter.release_focus()

func open_window(coords: Vector2i):
	popup()
	current_recipes = null
	current_coords = coords
	request_populate_recipes.rpc_id(1, current_coords)

func _on_search_bar_text_changed(_new_text):
	request_populate_recipes.rpc_id(1, current_coords)

@rpc("any_peer", "call_local", "unreliable")
func request_populate_recipes(coords: Vector2i):
	var new_recipes: Array = recipe.get_set_recipes().duplicate()
	var extra_recipes: Array = terminal_map.get_available_primary_recipes(coords)
	for obj: Array in extra_recipes:
		new_recipes.append(obj)
	populate_recipes.rpc_id(multiplayer.get_remote_sender_id(), new_recipes)

@rpc("authority", "call_local", "unreliable")
func populate_recipes(recipes_array: Array):
	current_recipes = recipes_array
	recipes.clear()
	for recipe_item: Array in current_recipes:
		var inputs = recipe_item[0]
		var outputs = recipe_item[1]
		if primary:
			populate_recipes_by_outputs(inputs, outputs)
		else:
			populate_recipes_by_inputs(inputs, outputs)

func populate_recipes_by_inputs(inputs: Dictionary, outputs: Dictionary):
	var filter_text = filter.text
	for input: String in inputs:
		if input.begins_with(filter_text):
			var recipe_str = get_name_for_recipe(inputs, outputs)
			recipes.add_item(recipe_str)
			break

func populate_recipes_by_outputs(inputs: Dictionary, outputs: Dictionary):
	var filter_text = filter.text
	for output: String in outputs:
		if output.begins_with(filter_text):
			var recipe_str = get_name_for_recipe(inputs, outputs)
			recipes.add_item(recipe_str)
			break

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

func _ready():
	hide()

func _on_close_requested():
	state_machine.unpress_gui()
	hide()

func _on_search_bar_focus_entered():
	state_machine.gui_button_pressed()

func _on_confirm_pressed():
	var selected_recipes: Array = recipes.get_selected_items()
	if !selected_recipes.is_empty():
		var selected_recipe: Array = current_recipes[selected_recipes[0]]
		request_change_construction_recipe.rpc_id(1, current_coords, selected_recipe)
		hide()

@rpc("any_peer", "call_local", "reliable")
func request_change_construction_recipe(coords: Vector2i, selected_recipe: Array):
	terminal_map.set_construction_site_recipe(coords, selected_recipe)

func _on_search_bar_focus_exited():
	state_machine.unpress_gui()

func _on_focus_exited():
	state_machine.unpress_gui()
	if filter != null:
		filter.release_focus()

func _on_switch_type_mouse_entered():
	timer = Timer.new()
	timer.one_shot = true
	add_child(timer)
	timer.timeout.connect(popup_hover_message)
	timer.start(0.66)

func _on_switch_type_mouse_exited():
	timer.queue_free()
	timer = null

func popup_hover_message():
	print("lol")

func _on_switch_type_pressed():
	primary = !primary
	open_window(current_coords)
