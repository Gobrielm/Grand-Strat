class_name ai_train extends train

var network_id: int

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("click") and visible:
		open_menu(map.get_mouse_local_to_map())

func get_last_stop() -> Vector2i:
	return stops.back()

func get_first_stop() -> Vector2i:
	return stops.front()
