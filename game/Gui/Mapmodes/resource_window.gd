extends Window

signal resource_window_picked

func _on_close_requested():
	hide()

func _on_item_list_item_selected(index):
	$ItemList.deselect_all()
	var real_index = terminal_map.get_cargo_type($ItemList.get_item_text(index))
	resource_window_picked.emit(real_index)
	hide()
