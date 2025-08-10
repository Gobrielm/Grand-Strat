extends Window
var location: Variant = null
var hold_name: String
var current_cargo: Dictionary
var current_prices: Dictionary
var current_cash: int
var current_level: int
var type_hovering: int = -1
var inside_price_list: bool = false

const time_every_update: int = 1
var progress: float = 0.0

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	hide()
# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	progress += delta
	if progress > time_every_update:
		refresh_window()
	refresh_hover()

func _on_close_requested() -> void:
	hide()

func open_window(new_location: Vector2i) -> void:
	location = new_location
	refresh_window()
	popup()

func refresh_window() -> void:
	progress = 0
	if location != null:
		request_current_cargo.rpc_id(1, location)
		request_current_name.rpc_id(1, location)
		request_current_prices.rpc_id(1, location)
		request_current_basic_labels.rpc_id(1, location)

func refresh_hover() -> void:
	if inside_price_list:
		var local_pos: Vector2 = $Price_Node/Price_List.get_local_mouse_position()
		var type: int = $Price_Node/Price_List.get_item_at_position(local_pos, true)
		
		start_hovering_type(type)
	else:
		$CargoInfoPopUp.hide()

func start_hovering_type(type: int) -> void:
	if type != type_hovering:
		$CargoInfoPopUp.start_hover()
		type_hovering = type

@rpc("any_peer", "call_local", "unreliable")
func request_current_cargo(coords: Vector2i) -> void:
	var dict: Dictionary = TerminalMap.get_instance().get_cargo_dict(coords)
	update_current_cargo.rpc_id(multiplayer.get_remote_sender_id(), dict)

@rpc("any_peer", "call_local", "unreliable")
func request_current_name(coords: Vector2i) -> void:
	var current_name: String = map_data.get_instance().get_hold_name(coords)
	update_current_name.rpc_id(multiplayer.get_remote_sender_id(), current_name)

@rpc("any_peer", "call_local", "unreliable")
func request_current_prices(coords: Vector2i) -> void:
	var dict: Dictionary = TerminalMap.get_instance().get_local_prices(coords)
	update_current_prices.rpc_id(multiplayer.get_remote_sender_id(), dict)

@rpc("any_peer", "call_local", "unreliable")
func request_current_basic_labels(coords: Vector2i) -> void:
	var broker: Broker = TerminalMap.get_instance().get_broker(coords)
	var _current_cash: int = TerminalMap.get_instance().get_cash_of_firm(coords)
	var level: int = 0
	if broker is FactoryTemplate:
		level = (TerminalMap.get_instance().get_broker(coords) as FactoryTemplate).get_level_without_employment()
	update_current_cash.rpc_id(multiplayer.get_remote_sender_id(), _current_cash)
	update_current_level.rpc_id(multiplayer.get_remote_sender_id(), level)
	update_connected_status.rpc_id(multiplayer.get_remote_sender_id(), broker.get_number_of_connected_terminals())

@rpc("any_peer", "call_local", "unreliable")
func request_current_level(coords: Vector2i) -> void:
	var broker: Broker = TerminalMap.get_instance().get_broker(coords)
	var _current_level: int = 0
	if broker is FactoryTemplate:
		_current_level = (TerminalMap.get_instance().get_broker(coords) as FactoryTemplate).get_level_without_employment()
	update_current_level.rpc_id(multiplayer.get_remote_sender_id(), _current_level)

@rpc("authority", "call_local", "unreliable")
func update_current_cargo(new_current_cargo: Dictionary) -> void:
	current_cargo = new_current_cargo
	factory_window()

@rpc("authority", "call_local", "unreliable")
func update_current_name(new_name: String) -> void:
	hold_name = new_name
	$Name.text = "[center][font_size=30]" + hold_name + "[/font_size][/center]"

@rpc("authority", "call_local", "unreliable")
func update_current_cash(new_cash: int) -> void:
	current_cash = new_cash
	$Cash.text = "$" + str(current_cash)

@rpc("authority", "call_local", "unreliable")
func update_current_prices(new_prices: Dictionary) -> void:
	current_prices = new_prices
	display_current_prices()

@rpc("authority", "call_local", "unreliable")
func update_current_level(new_level: int) -> void:
	current_level = new_level
	$Level.text = "Level: " + str(current_level)

@rpc("authority", "call_local", "unreliable")
func update_connected_status(connections: int) -> void:
	$Connected.text = "Connections: " + str(connections);

func factory_window() -> void:
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	var names: Array = CargoInfo.get_instance().get_cargo_array()
	var selected_name: String = get_selected_name()
	
	for i: int in cargo_list.item_count:
		cargo_list.remove_item(0)
	for cargo: int in current_cargo.size():
		if current_cargo[cargo] != 0:
			var cargo_name: String = names[cargo]
			cargo_list.add_item(cargo_name + ", " + str(current_cargo[cargo]))
			if cargo_name == selected_name:
				cargo_list.select(cargo)

func display_current_prices() -> void:
	var price_list: ItemList = $Price_Node/Price_List
	price_list.clear()
	var names: Array = CargoInfo.get_instance().get_cargo_array()
	for type: int in current_prices:
		price_list.add_item(names[type] + ": " + str(current_prices[type]))

func get_selected_name() -> String:
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	var selected_items: Array = cargo_list.get_selected_items()
	if selected_items.size() > 0:
		var toReturn: String = ""
		for i: String in cargo_list.get_item_text(selected_items[0]):
			if i == ',':
				break
			toReturn += i
		return toReturn
	return ""

func _on_price_list_mouse_entered() -> void:
	inside_price_list = true

func _on_price_list_mouse_exited() -> void:
	inside_price_list = false
	type_hovering = -1
	$CargoInfoPopUp.stop_hover()

func _on_cargo_info_pop_up_popup_requested() -> void:
	populate_info_window.rpc_id(1, type_hovering, location)

@rpc("any_peer", "call_local", "unreliable")
func populate_info_window(type: int, p_location: Vector2i) -> void:
	var info: Dictionary = {}
	var terminal_map: TerminalMap = TerminalMap.get_instance()
	var factory: Factory = terminal_map.get_factory(p_location)
	info.type = CargoInfo.get_instance().get_cargo_name(type)
	info.price = "$" + str(Utils.round(factory.get_local_price(type), 2))
	info.amount = "Amount: " + str(Utils.round(factory.get_cargo_amount(type), 2))
	info.market_info = "Supply: " + str(Utils.round(factory.get_last_month_supply()[type], 2)) + '\n' + "Demand: " + str(Utils.round(factory.get_last_month_demand()[type], 2))
	pop_up_info_window.rpc_id(multiplayer.get_remote_sender_id(), info)

@rpc("authority", "call_local", "unreliable")
func pop_up_info_window(info: Dictionary) -> void:
	var pop_up: cargo_info_popup = $CargoInfoPopUp
	pop_up.pop_up_info_window(info, get_mouse_position() + Vector2(position))
