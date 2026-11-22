extends Window
var location: Variant = null
var hold_name: String

var current_cash: int
var current_level: int
var type_hovering: int = -1
var inside_price_list: bool = false

var unformatted_farm_information: Dictionary = {}

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
		request_farm_information.rpc_id(1, location)

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
func request_farm_information(coords: Vector2i) -> void:
	var unformatted_info: Dictionary = ProvinceManager.get_instance().get_subsistence_farm_info(coords)
	update_farm_infromation.rpc_id(multiplayer.get_remote_sender_id(), unformatted_info)

@rpc("authority", "call_local", "unreliable")
func update_connected_status(connections: int) -> void:
	$Connected.text = "Connections: " + str(connections)

@rpc("authority", "call_local", "unreliable")
func update_farm_infromation(unformatted_info: Dictionary) -> void:
	unformatted_farm_information = unformatted_info
	$Cash.text = "Cash: " + str(Utils.round(unformatted_info.get("cash", 0), 1))
	$Level.text = "Level: " + str(Utils.round(unformatted_info.get("level", 0), 1))
	$UnemploymentRate.text = "U-rate: " + str(Utils.round(100.0 - unformatted_info.get("employment_rate", 0) * 100)) + "%"
	farm_window()

func farm_window() -> void:
	var cargo_list: ItemList = $Cargo_Node/Cargo_List
	var names: Array = CargoInfo.get_instance().get_cargo_array()
	#var selected_name: String = get_selected_name()
	
	for i: int in cargo_list.item_count:
		cargo_list.remove_item(0)
	var cargo_dict: Dictionary = unformatted_farm_information.get("cargo", {})
	for type: int in cargo_dict:
		if cargo_dict[type].amount != 0:
			var cargo_name: String = names[type]
			cargo_list.add_item(cargo_name + ", " + str(cargo_dict[type].amount))
			#if cargo_name == selected_name:
				#cargo_list.select(type)
	display_current_prices()

func display_current_prices() -> void:
	var price_list: ItemList = $Price_Node/Price_List
	price_list.clear()
	var names: Array = CargoInfo.get_instance().get_cargo_array()
	var cargo_dict: Dictionary = unformatted_farm_information.get("cargo", {})
	
	for type: int in cargo_dict:
		var pdh: PDH = PDH.new()
		pdh.price = cargo_dict[type].price
		pdh.demand = cargo_dict[type].demand
		pdh.supply = cargo_dict[type].supply
		price_list.add_item(names[type] + ": " + str(pdh.to_string()))

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
	if (!unformatted_farm_information.has(type)): return
	
	var info: Dictionary = {}
	info.type = CargoInfo.get_instance().get_cargo_name(type)
	info.price = "$" + str(Utils.round(unformatted_farm_information[type].price, 2))
	info.supply = "Supply: " + str(unformatted_farm_information[type].supply)
	info.demand = "Demand: " + str(unformatted_farm_information[type].demand)
	
	pop_up_info_window(info)

func pop_up_info_window(info: Dictionary) -> void:
	var pop_up: cargo_info_popup = $CargoInfoPopUp
	pop_up.pop_up_info_window(info, get_mouse_position() + Vector2(position))
