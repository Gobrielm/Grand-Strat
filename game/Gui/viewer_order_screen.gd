extends Control

@onready var station_window: Window = get_parent()

var buy_icon: CompressedTexture2D = preload("res://Gui/Icons/buy.png")
var sell_icon: CompressedTexture2D = preload("res://Gui/Icons/sell.png")

var current_orders: Dictionary

func update_orders(new_current_orders: Dictionary) -> void:
	current_orders = {}
	for type: int in new_current_orders:
		current_orders[type] = trade_order.construct_from_array(new_current_orders[type])
	update_order_screen()

func update_order_screen() -> void:
	clear_orders()
	for type: int in current_orders:
		var order: trade_order = current_orders[type]
		create_order_locally(order.get_type(), order.get_amount(), order.is_buy_order(), order.max_price)

func clear_orders() -> void:
	$Cargo_List.clear()

func edit_order(index: int, type: int, amount: int, buy: bool) -> void:
	set_order_icon(index, buy)
	$Cargo_List.set_item_text(index, terminal_map.get_cargo_name(type) + ": " + str(amount))

func create_order_locally(type: int, amount: int, buy: bool, price: float) -> void:
	$Cargo_List.add_item(terminal_map.get_cargo_name(type) + ": " + str(amount) + " -- " + str(price))
	set_order_icon($Cargo_List.item_count - 1, buy)

func set_order_icon(index: int, buy: bool) -> void:
	if buy:
		$Cargo_List.set_item_icon(index, buy_icon)
	else:
		$Cargo_List.set_item_icon(index, sell_icon)
