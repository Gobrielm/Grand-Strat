extends Control

@onready var station_window: Window = get_parent()

var buy_icon: CompressedTexture2D = preload("res://Gui/Icons/buy.png")
var sell_icon: CompressedTexture2D = preload("res://Gui/Icons/sell.png")

var current_orders: Dictionary

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("delete"):
		var index: int = get_selected_item()
		if index != -1:
			remove_order(index)
		

func get_selected_item() -> int:
	var items: Array = $Cargo_List.get_selected_items()
	if items.size() == 0:
		return -1
	else:
		return items[0]

func update_orders(new_current_orders: Dictionary) -> void:
	current_orders = {}
	for type: int in new_current_orders:
		current_orders[type] = trade_order.construct_from_array(new_current_orders[type])
	update_order_screen()

func update_order_screen() -> void:
	var type_selected: int = get_selected_type()
	clear_orders()
	for type: int in current_orders:
		var order: trade_order = current_orders[type]
		create_order_locally(order.get_type(), order.get_amount(), order.is_buy_order())
		if type == type_selected:
			$Cargo_List.select($Cargo_List.item_count - 1)

func clear_orders() -> void:
	$Cargo_List.clear()

func _on_add_order_pressed() -> void:
	$Order_Window.popup()

func _on_order_window_placed_order(type: int, amount: int, buy: bool, price: float) -> void:
	var location: Vector2i = station_window.get_location()
	#Check to see if order exists first
	for cargo_type: int in terminal_map.get_instance().get_station_orders(location):
		if cargo_type == type:
			edit_order(cargo_type, type, amount, buy)
			terminal_map.get_instance().edit_order_station(location, type, amount, buy, price)
			return
	create_order_locally(type, amount, buy)
	terminal_map.get_instance().edit_order_station(location, type, amount, buy, price)

func edit_order(index: int, type: int, amount: int, buy: bool) -> void:
	set_order_icon(index, buy)
	$Cargo_List.set_item_text(index, terminal_map.get_instance().get_cargo_name(type) + ": " + str(amount))

func create_order_locally(type: int, amount: int, buy: bool) -> void:
	$Cargo_List.add_item(terminal_map.get_instance().get_cargo_name(type) + ": " + str(amount))
	set_order_icon($Cargo_List.item_count - 1, buy)

func set_order_icon(index: int, buy: bool) -> void:
	if buy:
		$Cargo_List.set_item_icon(index, buy_icon)
	else:
		$Cargo_List.set_item_icon(index, sell_icon)

func remove_order(index: int) -> void:
	var text: String = $Cargo_List.get_item_text(index)
	while text.length() > 0:
		var arg: String = text.right(1)
		if !arg.is_valid_int() and !arg == ":" and !arg == " ":
			break
		text = text.left(text.length() - 1)
	
	var location: Vector2i = station_window.get_location()
	var type: int = terminal_map.get_instance().get_cargo_type(text)
	terminal_map.get_instance().remove_order_station(location, type)
	$Cargo_List.remove_item(index)

func get_selected_type() -> int:
	var index: int = get_selected_item()
	if index == -1:
		return -1
	var text: String = $Cargo_List.get_item_text(index)
	while text.length() > 0:
		var arg: String = text.right(1)
		if !arg.is_valid_int() and !arg == ":" and !arg == " ":
			break
		text = text.left(text.length() - 1)
	return terminal_map.get_instance().get_cargo_type(text)
