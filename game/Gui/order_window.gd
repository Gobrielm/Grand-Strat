extends Window

signal placed_order

var completed = false

# Called when the node enters the scene tree for the first time.
func _ready():
	hide()

func _on_about_to_popup():
	if !completed:
		var cargo_list: ItemList = $Control/Cargo_List
		for type: String in terminal_map.get_cargo_array():
			cargo_list.add_item(type)
		completed = true

func _on_buy_button_pressed():
	$Sell_Button.set_pressed_no_signal(!$Sell_Button.button_pressed)

func _on_sell_button_pressed():
	$Buy_Button.set_pressed_no_signal(!$Buy_Button.button_pressed)

func get_selected_item() -> int:
	var items: Array = $Control/Cargo_List.get_selected_items()
	if items.size() == 0:
		return -1
	else:
		return items[0]

func _on_place_order_pressed():
	hide()
	var type = get_selected_item()
	if type == -1:
		return
	$Control/Cargo_List.deselect(type)
	var amount = $Amount.value
	if amount <= 0:
		return
	var buy = $Buy_Button.button_pressed
	placed_order.emit(type, amount, buy)


func _on_close_requested():
	hide()
