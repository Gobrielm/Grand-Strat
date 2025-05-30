extends Window
var location: Variant = null
var hold_name: String
var current_cargo: Dictionary
var current_cash: int

const time_every_update: int = 1
var progress: float = 0.0

@onready var order_screen: Control = $Viewer_order_screen

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	progress += delta
	if progress > time_every_update:
		refresh_window()

func _on_close_requested() -> void:
	hide()

func open_window(new_location: Vector2i) -> void:
	location = new_location
	refresh_window()
	popup()

func get_location() -> Vector2i:
	return location

func refresh_window() -> void:
	progress = 0
	if location != null:
		request_station_cargo.rpc_id(1, location)
		request_current_name.rpc_id(1, location)
		request_current_cash.rpc_id(1, location)
		request_current_orders.rpc_id(1, location)
		station_window()

@rpc("any_peer", "call_local", "unreliable")
func request_current_name(coords: Vector2i) -> void:
	var current_name: String = map_data.get_instance().get_hold_name(coords)
	update_current_name.rpc_id(multiplayer.get_remote_sender_id(), current_name)

@rpc("any_peer", "call_local", "unreliable")
func request_station_cargo(coords: Vector2i) -> void:
	var good_dict: Dictionary = terminal_map.get_instance().get_cargo_array_at_location(coords)
	update_current_cargo.rpc_id(multiplayer.get_remote_sender_id(), good_dict)

@rpc("any_peer", "call_local", "unreliable")
func request_current_cash(coords: Vector2i) -> void:
	var _current_cash: int = terminal_map.get_instance().get_cash_of_firm(coords)
	update_current_cash.rpc_id(multiplayer.get_remote_sender_id(), _current_cash)

@rpc("any_peer", "call_local", "unreliable")
func request_current_orders(coords: Vector2i) -> void:
	var current_orders: Dictionary = terminal_map.get_instance().get_station_orders(coords)
	update_current_orders.rpc_id(multiplayer.get_remote_sender_id(), current_orders)

@rpc("authority", "call_local", "unreliable")
func update_current_cargo(new_cargo_dict: Dictionary) -> void:
	current_cargo = new_cargo_dict

@rpc("authority", "call_local", "unreliable")
func update_current_name(new_name: String) -> void:
	$Name.text = "[center][font_size=30]" + new_name + "[/font_size][/center]"

@rpc("authority", "call_local", "unreliable")
func update_current_cash(new_cash: int) -> void:
	current_cash = new_cash
	$Cash.text = "$" + str(current_cash)

@rpc("authority", "call_local", "unreliable")
func update_current_orders(new_current_orders: Dictionary) -> void:
	order_screen.update_orders(new_current_orders)

func station_window() -> void:
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	for i: int in cargo_list.item_count:
		cargo_list.remove_item(0)
	for cargo: int in current_cargo.size():
		if current_cargo[cargo] != 0:
			var cargo_name: String = terminal_map.get_instance().get_cargo_name(cargo)
			cargo_list.add_item(cargo_name + ", " + str(current_cargo[cargo]))
