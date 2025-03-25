extends Window
@onready var train = get_parent()

# Called when the node enters the scene tree for the first time.
func _ready():
	pass # Replace with function body.

func _input(event):
	if event.is_action_pressed("delete"):
		var routes: ItemList = $Routes
		#Should only every be 1 item selected so this works fine but break just in case
		for index in routes.get_selected_items():
			train.remove_stop.rpc(index)
			break
	elif event.is_action_pressed("deselect"):
		deselect_add_stop()

#func is_mouse_hovering() -> bool:
	#var state = false
	#print(gui_get_hovered_control())
	#return state


func deselect_add_stop():
	$Routes/Add_Stop.button_pressed = false
	state_machine.stop_selecting_route()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(_delta):
	pass

func _on_add_stop_pressed():
	state_machine.start_selecting_route()
	
