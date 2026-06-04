extends Window
var location: Variant = null
var hold_name: String

var town_pdps: Dictionary

var current_cash: int
var current_level: int
var type_hovering: int = -1
var inside_price_list: bool = false

var unformatted_factory_information: Dictionary = {}

const time_every_update: int = 1
var progress: float = 0.0

func _ready() -> void:
	hide()

func _process(delta: float) -> void:
	if !visible:
		return
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
		request_current_name.rpc_id(1, location)
		request_current_prices.rpc_id(1, location)
		request_factory_information.rpc_id(1, location)

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
func request_current_name(coords: Vector2i) -> void:
	var current_name: String = map_data.get_instance().get_hold_name(coords)
	update_current_name.rpc_id(multiplayer.get_remote_sender_id(), current_name)

@rpc("any_peer", "call_local", "unreliable")
func request_current_prices(coords: Vector2i) -> void:
	var dict: Dictionary = ProvinceManager.get_instance().get_town_pdps(coords)
	update_current_prices.rpc_id(multiplayer.get_remote_sender_id(), dict)

@rpc("any_peer", "call_local", "unreliable")
func request_factory_information(coords: Vector2i) -> void:
	var unformatted_info: Dictionary = ProvinceManager.get_instance().get_factory_info(coords)
	update_factory_infromation.rpc_id(multiplayer.get_remote_sender_id(), unformatted_info)

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
	town_pdps = new_prices
	display_current_prices()

@rpc("authority", "call_local", "unreliable")
func update_connected_status(connections: int) -> void:
	$Connected.text = "Connections: " + str(connections)

@rpc("authority", "call_local", "unreliable")
func update_factory_infromation(unformatted_info: Dictionary) -> void:
	unformatted_factory_information = unformatted_info
	$Cash.text = "Cash: " + str(unformatted_info.get("cash", 0))
	$Level.text = "Level: " + str(unformatted_info.get("level", 0))
	$UnemploymentRate.text = "U-rate: " + str(100.0 - unformatted_info.get("employment_rate", 0) * 100) + "%"
	factory_window()

func factory_window() -> void:
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	var names: Array = CargoInfo.get_instance().get_cargo_array()
	#var selected_name: String = get_selected_name()
	
	for i: int in cargo_list.item_count:
		cargo_list.remove_item(0)
	var cargo_dict: Dictionary = unformatted_factory_information.get("cargo", {})
	for type: int in cargo_dict:
		if cargo_dict[type].amount != 0:
			var cargo_name: String = names[type]
			cargo_list.add_item(cargo_name + ", " + str(cargo_dict[type].amount))
			#if cargo_name == selected_name:
				#cargo_list.select(type)

func display_current_prices() -> void:
	var price_list: ItemList = $Price_Node/Price_List
	price_list.clear()
	var names: Array = CargoInfo.get_instance().get_cargo_array()
	for type: int in town_pdps:
		var pdp: PDP = town_pdps[type]
		price_list.add_item(names[type] + ": " + str(pdp.to_string()))

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
	populate_info_window(type_hovering)

func populate_info_window(type: int) -> void:
	if (!unformatted_factory_information.has(type)): return
	
	var info: Dictionary = {}
	info.type = CargoInfo.get_instance().get_cargo_name(type)
	info.price = "$" + str(Utils.round(unformatted_factory_information[type].price, 2))
	info.supply = "Supply: " + str(unformatted_factory_information[type].supply)
	info.demand = "Demand: " + str(unformatted_factory_information[type].demand)
	
	pop_up_info_window(info)

func pop_up_info_window(info: Dictionary) -> void:
	var pop_up: cargo_info_popup = $CargoInfoPopUp
	pop_up.pop_up_info_window(info, get_mouse_position() + Vector2(position))
