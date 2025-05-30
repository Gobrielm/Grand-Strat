extends Window

signal resource_window_picked

func _on_close_requested() -> void:
	hide()

func _on_item_list_item_selected(index: int) -> void:
	$ItemList.deselect_all()
	var real_index: int = terminal_map.get_instance().get_cargo_type($ItemList.get_item_text(index))
	resource_window_picked.emit(real_index)
	hide()
