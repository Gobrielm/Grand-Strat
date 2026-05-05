class_name cargo_info_popup extends Popup

signal popup_requested

func pop_up_info_window(info: Dictionary, position_to_popup: Vector2) -> void:
	position = position_to_popup
	get_node("Name").text = info.type
	get_node("Price").text = info.price
	get_node("Supply").text = info.supply
	get_node("Demand").text = info.demand
	popup()
	position = position_to_popup

func start_hover() -> void:
	$hover_timer.start(0.5)

func stop_hover() -> void:
	$hover_timer.stop()

func _on_hover_timer_timeout() -> void:
	popup_requested.emit()
