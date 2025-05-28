extends Sprite2D

@onready var window: Window = $Window
@onready var map: TileMapLayer = get_parent()

func _ready() -> void:
	window.hide()

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("click") and is_selecting_route():
		do_add_stop.rpc_id(1, map.get_cell_position())
	elif event.is_action_pressed("click") and visible:
		open_menu(map.get_mouse_local_to_map())
	elif event.is_action_pressed("deselect"):
		window.deselect_add_stop()

func start_selecting_route() -> void:
	province_machine.start_selecting_route()

func stop_selecting_route() -> void:
	province_machine.stop_selecting_route()

func is_selecting_route() -> bool:
	return province_machine.is_selecting_route()

func did_mouse_click(mouse_pos: Vector2) -> bool:
	var match_x: bool = mouse_pos.x > position.x - 48 and mouse_pos.x < position.x + 48
	var match_y: bool = mouse_pos.y > position.y - 48 and mouse_pos.y < position.y + 48
	return match_x and match_y

func open_menu(mouse_pos: Vector2) -> void:
	if did_mouse_click(mouse_pos):
		window.popup()

func _on_window_close_requested() -> void:
	window.deselect_add_stop()
	window.hide()

func _on_start_pressed() -> void:
	start_train.rpc_id(1)

func _on_stop_pressed() -> void:
	stop_train.rpc_id(1)

@rpc("any_peer", "unreliable", "call_local")
func start_train() -> void:
	pass

@rpc("any_peer", "unreliable", "call_local")
func stop_train() -> void:
	pass

@rpc("any_peer", "unreliable", "call_local")
func do_add_stop() -> void:
	pass

@rpc("authority", "unreliable", "call_local")
func update_train(new_position: Vector2i) -> void:
	position = new_position

@rpc("authority", "unreliable", "call_local")
func update_train_rotation(new_rotation: int) -> void:
	rotation_degrees = new_rotation

@rpc("authority", "unreliable", "call_local")
func create(new_location: Vector2i) -> void:
	position = map.map_to_local(new_location)

@rpc("authority", "unreliable", "call_local")
func add_stop(new_location: Vector2i, add_to_start: bool = false) -> void:
	var routes: ItemList = $Window/Routes
	var index: int = routes.add_item(str(new_location))
	if add_to_start:
		routes.move_item(index, 0)

@rpc("any_peer", "unreliable", "call_local")
func remove_stop(index: int) -> void:
	var routes: ItemList = $Window/Routes
	routes.remove_item(index)

@rpc("authority", "unreliable", "call_local")
func update_cargo_gui(names: Array, amounts: Array) -> void:
	$Window/Goods.clear()
	for type: int in names.size():
		$Window/Goods.add_item(names[type] + ", " + str(amounts[type]))

@rpc("authority", "unreliable", "call_local")
func go_into_depot() -> void:
	visible = false

@rpc("authority", "unreliable", "call_local")
func go_out_of_depot(new_dir: int) -> void:
	visible = true
	rotation = new_dir * 60
