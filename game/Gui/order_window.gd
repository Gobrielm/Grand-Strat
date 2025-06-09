extends Window

signal placed_order

var completed: bool = false

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()

func _on_about_to_popup() -> void:
	if !completed:
		var cargo_list: ItemList = $Control/Cargo_List
		for type: String in TerminalMap.get_instance().get_cargo_array():
			cargo_list.add_item(type)
		completed = true

func _on_buy_button_pressed() -> void:
	$Sell_Button.set_pressed_no_signal(!$Sell_Button.button_pressed)

func _on_sell_button_pressed() -> void:
	$Buy_Button.set_pressed_no_signal(!$Buy_Button.button_pressed)

func get_selected_item() -> int:
	var items: Array = $Control/Cargo_List.get_selected_items()
	if items.size() == 0:
		return -1
	else:
		return items[0]

func _on_place_order_pressed() -> void:
	hide()
	var type: int = get_selected_item()
	if type == -1:
		return
	$Control/Cargo_List.deselect(type)
	var amount: int = $Amount.value
	if amount <= 0:
		return
	var buy: bool = $Buy_Button.button_pressed
	var price: float = $Price.value
	if price == 0.0:
		return
	placed_order.emit(type, amount, buy, price)

func _on_close_requested() -> void:
	hide()
