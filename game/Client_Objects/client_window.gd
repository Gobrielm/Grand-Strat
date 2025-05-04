extends Window

@onready var train_obj: train = get_parent()

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("delete"):
		var routes: ItemList = $Routes
		#Should only every be 1 item selected so this works fine but break just in case
		for index: int in routes.get_selected_items():
			train_obj.remove_stop.rpc(index)
			break
	elif event.is_action_pressed("deselect"):
		deselect_add_stop()

func deselect_add_stop() -> void:
	$Routes/Add_Stop.button_pressed = false
	train_obj.stop_selecting_route()

func _on_add_stop_pressed() -> void:
	train_obj.start_selecting_route()
