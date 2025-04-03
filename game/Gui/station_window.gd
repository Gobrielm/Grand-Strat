extends Window
var location = null
var hold_name: String
var current_cargo: Dictionary
var current_cash: int

const time_every_update = 1
var progress: float = 0.0

@onready var order_screen = $Order_Screen

# Called when the node enters the scene tree for the first time.
func _ready():
	hide()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	progress += delta
	if progress > time_every_update:
		refresh_window()

func _on_close_requested():
	hide()

func open_window(new_location: Vector2i):
	location = new_location
	refresh_window()
	popup()

func get_location() -> Vector2i:
	return location

func refresh_window():
	progress = 0
	if location != null:
		request_station_cargo.rpc_id(1, location)
		request_current_name.rpc_id(1, location)
		request_current_cash.rpc_id(1, location)
		request_current_orders.rpc_id(1, location)
		station_window()

@rpc("any_peer", "call_local", "unreliable")
func request_current_name(coords: Vector2i):
	var current_name = map_data.get_instance().get_instance().get_hold_name(coords)
	update_current_name.rpc_id(multiplayer.get_remote_sender_id(), current_name)

@rpc("any_peer", "call_local", "unreliable")
func request_station_cargo(coords: Vector2i):
	var good_dict = terminal_map.get_cargo_array_at_location(coords)
	update_current_cargo.rpc_id(multiplayer.get_remote_sender_id(), good_dict)

@rpc("any_peer", "call_local", "unreliable")
func request_current_cash(coords: Vector2i):
	var _current_cash = terminal_map.get_cash_of_firm(coords)
	update_current_cash.rpc_id(multiplayer.get_remote_sender_id(), _current_cash)

@rpc("any_peer", "call_local", "unreliable")
func request_current_orders(coords: Vector2i):
	var current_orders = terminal_map.get_station_orders(coords)
	update_current_orders.rpc_id(multiplayer.get_remote_sender_id(), current_orders)

@rpc("authority", "call_local", "unreliable")
func update_current_cargo(new_cargo_dict: Dictionary):
	current_cargo = new_cargo_dict

@rpc("authority", "call_local", "unreliable")
func update_current_name(new_name: String):
	$Name.text = "[center][font_size=30]" + new_name + "[/font_size][/center]"

@rpc("authority", "call_local", "unreliable")
func update_current_cash(new_cash: int):
	current_cash = new_cash
	$Cash.text = "$" + str(current_cash)

@rpc("authority", "call_local", "unreliable")
func update_current_orders(new_current_orders: Dictionary):
	order_screen.update_orders(new_current_orders)

func station_window():
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	var selected_name = get_selected_name()
	for i in cargo_list.item_count:
		cargo_list.remove_item(0)
	for cargo in current_cargo.size():
		if current_cargo[cargo] != 0:
			var cargo_name: String = terminal_map.get_cargo_name(cargo)
			cargo_list.add_item(cargo_name + ", " + str(current_cargo[cargo]))
			if cargo_name == selected_name:
				cargo_list.select(cargo)


func get_selected_name() -> String:
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	var selected_items: Array = cargo_list.get_selected_items()
	if selected_items.size() > 0:
		var toReturn = ""
		for i in cargo_list.get_item_text(selected_items[0]):
			if i == ',':
				break
			toReturn += i
		return toReturn
	return ""
