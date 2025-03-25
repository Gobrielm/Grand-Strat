extends Window

@onready var train = get_parent()

func _input(event):
	if event.is_action_pressed("delete"):
		var routes: ItemList = $Routes
		#Should only every be 1 item selected so this works fine but break just in case
		for index in routes.get_selected_items():
			train.remove_stop.rpc(index)
			break
	elif event.is_action_pressed("deselect"):
		deselect_add_stop()

func deselect_add_stop():
	$Routes/Add_Stop.button_pressed = false
	train.stop_selecting_route()

func _on_add_stop_pressed():
	train.start_selecting_route()
